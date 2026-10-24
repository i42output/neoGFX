// library.cpp
/*
neoGFX Resource Compiler
Copyright(C) 2019 Leigh Johnston

This program is free software: you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <neogfx/neogfx.hpp>

#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <streambuf>
#include <istream>
#include <ostream>
#include <optional>
#include <utility>
#include <map>
#include <vector>
#include <string_view>
#include <filesystem>
#include <cctype>
#include <algorithm>

#include <boost/lexical_cast.hpp>

#include <neolib/core/string_ci.hpp>
#include <neolib/core/service.hpp>
#include <neolib/task/timer.hpp>

#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gfx/utility.hpp>
#include <neogfx/tools/DesignStudio/i_project.hpp>
#include <neogfx/tools/DesignStudio/element.hpp>
#include <neogfx/tools/DesignStudio/console_client.hpp>
#include <neogfx/tools/DesignStudio/i_script_highlighter.hpp>
#include <neos/context.hpp>
#include <neos/console.hpp>
#include <neos/language/schema.hpp>
#include "library.hpp"

namespace neogfx::DesignStudio
{
    // a neos session in a console window (the "neos" command): the neos console's commands (see neos::process_command) on a neos
    // context whose standard streams are the window's (the output of the context and of the programs it runs goes to the window; 
    // what is typed into the window while a command runs is its programs' input). Commands run on a worker thread (each session 
    // has its own, and its own context), so the GUI thread never blocks: it only exchanges data with the worker through 
    // shared_state (under a mutex held only for brief copies) and polls for the worker's output with a timer (as the ssh client).
    class neos_session : public console_client
    {
    private:
        struct shared_state
        {
            // (the context's output and error streams: to the window (output, below), in the order written, from any thread)
            class output_buffer : public std::streambuf
            {
            public:
                output_buffer(shared_state& aShared) : iShared{ aShared } {}
            protected:
                int_type overflow(int_type aCharacter) override
                {
                    if (!traits_type::eq_int_type(aCharacter, traits_type::eof()))
                    {
                        std::lock_guard<std::mutex> lock{ iShared.mutex };
                        iShared.output.push_back(traits_type::to_char_type(aCharacter));
                    }
                    return traits_type::not_eof(aCharacter);
                }
                std::streamsize xsputn(char const* aText, std::streamsize aCount) override
                {
                    std::lock_guard<std::mutex> lock{ iShared.mutex };
                    iShared.output.append(aText, static_cast<std::size_t>(aCount));
                    return aCount;
                }
            private:
                shared_state& iShared;
            };
            // (the context's input stream: the lines typed into the window while a command runs (input, below); a read waits for 
            // one; the end of input (Ctrl+Z or Ctrl+D) is read once)
            class input_buffer : public std::streambuf
            {
            public:
                input_buffer(shared_state& aShared) : iShared{ aShared } {}
            public:
                void reset()
                {
                    iBuffer.clear();
                    setg(nullptr, nullptr, nullptr);
                }
            protected:
                int_type underflow() override
                {
                    if (gptr() < egptr())
                        return traits_type::to_int_type(*gptr());
                    std::unique_lock<std::mutex> lock{ iShared.mutex };
                    iShared.wake.wait(lock, [&]() { return !iShared.input.empty() || iShared.endOfInput || iShared.cancelled; });
                    if (iShared.input.empty())
                    {
                        iShared.endOfInput = false;
                        return traits_type::eof();
                    }
                    // (the last character read is kept, so that it can be put back)
                    bool const kept = eback() != nullptr && gptr() > eback();
                    char const last = kept ? gptr()[-1] : '\0';
                    iBuffer.assign(kept ? 1u : 0u, last);
                    iBuffer += iShared.input;
                    iShared.input.clear();
                    setg(iBuffer.data(), iBuffer.data() + (kept ? 1 : 0), iBuffer.data() + iBuffer.size());
                    return traits_type::to_int_type(*gptr());
                }
            private:
                shared_state& iShared;
                std::string iBuffer;
            };

            std::mutex mutex;
            std::condition_variable wake;
            bool cancelled = false;
            // GUI thread -> worker thread
            std::optional<std::string> command;
            std::string input;
            bool endOfInput = false;
            // worker thread -> GUI thread
            std::string output;
            std::string prompt;
            bool busy = false;      ///< (a command is running: lines typed are its programs' input)
            bool closed = false;
            bool runOnly = false;   ///< (programs are run (each command is a program's path): only their input and output is shown)
            // (the context's standard streams (destroyed after it))
            output_buffer outputBuffer{ *this };
            input_buffer inputBuffer{ *this };
            std::ostream out{ &outputBuffer };
            std::istream in{ &inputBuffer };
            std::optional<neos::context> context;
        };
    public:
        // (a program, if any: the session only runs programs (it and those that it is asked to run (see execute)), showing only 
        // their input and output: it has no prompt, and nothing typed while none is running is a command (the "neos <program>" command))
        neos_session(neolib::i_service_provider& aServiceProvider, std::string const& aProgram = {}) :
            iShared{ std::make_shared<shared_state>() }
        {
            if (!aProgram.empty())
            {
                iShared->runOnly = true;
                iShared->command = aProgram;
                iShared->busy = true;
            }
            try
            {
                iShared->in.tie(&iShared->out);
                iShared->context.emplace(iShared->in, iShared->out, iShared->out, aServiceProvider);
                // The worker thread is detached and never joined (a program it is running may not end); it owns everything it 
                // touches (via shared_state) and exits once its command has finished if the session has been closed.
                std::thread{ [shared = iShared]() { run(shared); } }.detach();
            }
            catch (std::exception const& e)
            {
                iShared->output = std::string{ "neos: " } + e.what() + "\n";
                iShared->closed = true;
            }
        }
        ~neos_session()
        {
            {
                std::lock_guard<std::mutex> lock{ iShared->mutex };
                iShared->cancelled = true;
            }
            iShared->wake.notify_all();
        }
    public:
        void start() final
        {
            iPoller.emplace(neolib::service<neolib::i_async_task>(), [this](neolib::callback_timer& aTimer)
            {
                destroyed_flag destroyed{ *this };
                poll();
                if (!destroyed && !iClosed)
                    aTimer.again();
            }, std::chrono::milliseconds{ 10 });
        }
        void resize_window(std::uint16_t aWidth, std::uint16_t aHeight) final
        {
        }
        bool execute(std::string const& aCommand) final
        {
            if (iClosed)
                return false;
            std::string prompt;
            {
                std::lock_guard<std::mutex> lock{ iShared->mutex };
                if (iShared->busy)
                    return false;
                if (iShared->runOnly)
                {
                    // ("run <program>": the program run (as the session's first was))
                    if (aCommand.rfind("run ", 0u) != 0u)
                        return false;
                    iShared->command = aCommand.substr(4u);
                    iShared->busy = true;
                }
                prompt = iShared->prompt;
            }
            if (iShared->runOnly)
            {
                iLine.clear();
                iShared->wake.notify_all();
                return true;
            }
            // (as typed (a line partly typed is discarded))
            Output((iLine.empty() ? std::string{} : "^C\r\n" + prompt) + aCommand + "\r\n");
            iLine.clear();
            enter(aCommand);
            return true;
        }
        void input(std::string const& aText) final
        {
            if (iClosed || aText.empty() || aText[0] == '\x1B') // (escape sequences (e.g. the cursor keys) are not edited)
                return;
            {
                // (a session that only runs programs: what is typed while none is running is ignored)
                std::lock_guard<std::mutex> lock{ iShared->mutex };
                if (iShared->runOnly && !iShared->busy)
                    return;
            }
            destroyed_flag destroyed{ *this };
            std::string echo;
            for (char ch : aText)
            {
                // (the terminal sends "\n" after "\r" for Enter)
                if (ch == '\n' && iLastInputWasCR)
                {
                    iLastInputWasCR = false;
                    continue;
                }
                iLastInputWasCR = ch == '\r';
                bool const repeatedTab = ch == '\t' && iLastInputWasTab;
                iLastInputWasTab = ch == '\t';
                if (ch == '\r' || ch == '\n')
                {
                    Output(echo + "\r\n");
                    echo.clear();
                    enter(std::exchange(iLine, {}));
                }
                else if (ch == '\x7F' || ch == '\b')
                {
                    if (!iLine.empty())
                    {
                        // (a character: a UTF-8 sequence)
                        while (iLine.size() > 1u && (static_cast<unsigned char>(iLine.back()) & 0xC0u) == 0x80u)
                            iLine.pop_back();
                        iLine.pop_back();
                        echo += "\b\x1B[K";
                    }
                }
                else if (ch == '\t')
                    echo += complete(repeatedTab);
                else if (ch == '\x03') // (Ctrl+C: the line is discarded (a running program is not interrupted))
                {
                    iLine.clear();
                    std::lock_guard<std::mutex> lock{ iShared->mutex };
                    echo += "^C\r\n" + (iShared->busy ? std::string{} : iShared->prompt);
                }
                else if ((ch == '\x04' || ch == '\x1A') && iLine.empty()) // (Ctrl+D or Ctrl+Z: the end of input (no command running: the end of the session))
                {
                    Output(echo + "\r\n");
                    echo.clear();
                    bool busy = false;
                    {
                        std::lock_guard<std::mutex> lock{ iShared->mutex };
                        busy = iShared->busy;
                        if (busy)
                            iShared->endOfInput = true;
                        else
                            iShared->cancelled = true;
                    }
                    iShared->wake.notify_all();
                    if (!busy)
                    {
                        iClosed = true;
                        Closed();
                        return; // *this may no longer exist
                    }
                }
                else if (static_cast<unsigned char>(ch) >= 0x20u)
                {
                    iLine.push_back(ch);
                    echo.push_back(ch);
                }
            }
            if (!destroyed && !echo.empty())
                Output(echo);
        }
    private:
        // GUI thread: a line typed: a command, or the input of the command running
        void enter(std::string const& aLine)
        {
            std::string prompt;
            {
                std::lock_guard<std::mutex> lock{ iShared->mutex };
                if (iShared->busy)
                    iShared->input += aLine + "\n";
                else if (!aLine.empty())
                {
                    iShared->command = aLine;
                    iShared->busy = true;
                }
                else
                    prompt = iShared->prompt;
            }
            iShared->wake.notify_all();
            if (!prompt.empty())
                Output(prompt);
        }
        // GUI thread: Tab (no command running): the path being typed (the line's last word, or what follows an unclosed quote) 
        // completed as far as the names of the files and directories that it may be the start of allow (one name: a directory's is 
        // followed by a separator, a quoted file's by its closing quote); a beep if there are none (or several and it is already as far as they allow: 
        // a second Tab lists them); the text to echo
        std::string complete(bool aList)
        {
            std::string prompt;
            {
                std::lock_guard<std::mutex> lock{ iShared->mutex };
                if (iShared->busy)
                    return {};
                prompt = iShared->prompt;
            }
#ifdef _WIN32
            char const* const separators = "/\\";
            bool const hidingDotNames = false;
            auto const same = [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b)); };
#else
            char const* const separators = "/";
            bool const hidingDotNames = true;
            auto const same = [](char a, char b) { return a == b; };
#endif
            bool const quoted = std::count(iLine.begin(), iLine.end(), '"') % 2 == 1;
            auto const wordStart = quoted ? iLine.rfind('"') + 1u : iLine.find_last_of(' ') == std::string::npos ? 0u : iLine.find_last_of(' ') + 1u;
            std::string const word = iLine.substr(wordStart);
            auto const lastSeparator = word.find_last_of(separators);
            std::string const directory = lastSeparator == std::string::npos ? std::string{} : word.substr(0u, lastSeparator + 1u);
            std::string const prefix = word.substr(directory.size());
            char const separator = lastSeparator != std::string::npos ? word[lastSeparator] : static_cast<char>(std::filesystem::path::preferred_separator);
            auto const utf8 = [](std::filesystem::path const& aPath) { auto const name = aPath.u8string(); return std::string{ name.begin(), name.end() }; };
            // (the names that the prefix may be of: those that start with it (not those that start with "." unless it does, except on 
            // Windows))
            std::vector<std::pair<std::string, bool>> matches;
            std::error_code ec;
            std::filesystem::path const searched = directory.empty() ? std::filesystem::path{ "." } : 
                std::filesystem::path{ std::u8string{ directory.begin(), directory.end() } };
            for (std::filesystem::directory_iterator entry{ searched, ec }, end; !ec && entry != end; entry.increment(ec))
            {
                auto const name = utf8(entry->path().filename());
                if (name.size() >= prefix.size() && std::equal(prefix.begin(), prefix.end(), name.begin(), same) && (!hidingDotNames || name[0] != '.' || (!prefix.empty() && prefix[0] == '.')))
                {
                    std::error_code isDirectoryError;
                    matches.emplace_back(name, entry->is_directory(isDirectoryError));
                }
            }
            if (matches.empty())
            {
                service<i_basic_services>().system_beep();
                return {};
            }
            // (as far as the names allow: their common prefix)
            std::string completion = matches[0].first;
            for (auto const& match : matches)
                completion.erase(static_cast<std::size_t>(std::mismatch(completion.begin(), completion.end(), match.first.begin(), match.first.end(), same).first - completion.begin()));
            if (matches.size() > 1u && completion.size() == prefix.size())
            {
                service<i_basic_services>().system_beep();
                if (!aList)
                    return {};
                std::sort(matches.begin(), matches.end());
                std::string list = "\r\n";
                for (auto const& match : matches)
                    list += match.first + (match.second ? std::string(1u, separator) : std::string{}) + "  ";
                return list + "\r\n" + prompt + iLine;
            }
            // (a name with a space in it is quoted)
            bool const quote = !quoted && completion.find(' ') != std::string::npos;
            if (matches.size() == 1u)
                completion += matches[0].second ? std::string(1u, separator) : quoted || quote ? "\"" : "";
            std::string const completed = (quote ? "\"" : "") + directory + completion;
            // (the word typed replaced (its case may differ from the name's) unless it is the start of it)
            std::string echo;
            if (completed.compare(0u, word.size(), word) == 0)
                echo = completed.substr(word.size());
            else
            {
                for (char ch : word)
                    if ((static_cast<unsigned char>(ch) & 0xC0u) != 0x80u)
                        echo += "\b";
                echo += "\x1B[K" + completed;
            }
            iLine = iLine.substr(0u, wordStart) + completed;
            return echo;
        }
        // GUI thread: collect the worker's output; never waits on the worker
        void poll()
        {
            std::string output;
            bool closed = false;
            {
                std::lock_guard<std::mutex> lock{ iShared->mutex };
                output.swap(iShared->output);
                closed = iShared->closed;
            }
            if (!output.empty())
            {
                // (the terminal's line ending: "\r\n")
                std::string text;
                text.reserve(output.size() + output.size() / 8u);
                for (char ch : output)
                {
                    if (ch == '\n' && !iLastOutputWasCR)
                        text.push_back('\r');
                    text.push_back(ch);
                    iLastOutputWasCR = ch == '\r';
                }
                destroyed_flag destroyed{ *this };
                Output(text);
                if (destroyed)
                    return;
            }
            if (closed)
            {
                iClosed = true;
                Closed();
                return; // *this may no longer exist
            }
        }
        // the worker thread: the session's commands, until it is closed (the quit command, or the session is cancelled)
        static void run(std::shared_ptr<shared_state> aShared)
        {
            auto& shared = *aShared;
            auto& context = *shared.context;
            bool interactive = false;
            auto end_command = [&](bool aClosed)
            {
                shared.out.flush();
                shared.inputBuffer.reset();
                shared.in.clear();
                auto const prompt = aClosed || shared.runOnly ? std::string{} : neos::console_prompt(context, interactive);
                std::lock_guard<std::mutex> lock{ shared.mutex };
                // (input that the command's programs did not read is discarded)
                shared.input.clear();
                shared.endOfInput = false;
                shared.output += prompt;
                shared.prompt = prompt;
                shared.busy = false;
                shared.closed = shared.closed || aClosed;
            };
            try
            {
                if (!shared.runOnly)
                {
                    shared.out << "neos " << neos::NEOS_VERSION << std::endl;
                    end_command(false);
                }
                for (;;)
                {
                    std::string command;
                    {
                        std::unique_lock<std::mutex> lock{ shared.mutex };
                        shared.wake.wait(lock, [&]() { return shared.command || shared.cancelled; });
                        if (shared.cancelled)
                            return;
                        command = std::move(*shared.command);
                        shared.command.reset();
                    }
                    bool more = true;
                    if (shared.runOnly)
                    {
                        // (the program (cleared: only its input and output shown), and its errors)
                        shared.out << "\x1B[3J" << std::flush;
                        try
                        {
                            context.load_program(command);
                            context.compile_program();
                            context.run();
                        }
                        catch (std::exception const& e)
                        {
                            shared.out << (std::string_view{ e.what() }.find("error:") == std::string_view::npos ? "Error: " : "") << e.what() << std::endl;
                        }
                    }
                    else if (!interactive && (command == "cls" || command == "clear"))
                        shared.out << "\x1B[3J";
                    else
                        more = neos::process_command(context, interactive, command, true);
                    end_command(!more);
                    if (!more)
                        return;
                }
            }
            catch (std::exception const& e)
            {
                shared.out << "neos: " << e.what() << std::endl;
                end_command(true);
            }
            catch (...)
            {
                shared.out << "neos: unknown error" << std::endl;
                end_command(true);
            }
        }
    private:
        std::shared_ptr<shared_state> iShared;
        std::optional<neolib::callback_timer> iPoller;
        std::string iLine;
        bool iLastInputWasCR = false;
        bool iLastInputWasTab = false;
        bool iLastOutputWasCR = false;
        bool iClosed = false;
    };

    // the syntax highlighting of scripts (see i_script_highlighter) in the languages that neos has (installed) schemas for (see 
    // neos::context::highlight). Calls are serialized (the neos context, for its concept libraries, compiler and the schemas' parsers, is 
    // used by one thread at a time).
    class neos_script_highlighter : public i_script_highlighter
    {
    public:
        bool can_highlight(neolib::i_string const& aExtension) const final
        {
            return schema_path(aExtension.to_std_string()).has_value();
        }
        void extensions(neolib::i_vector<neolib::i_string>& aExtensions) const final
        {
            aExtensions.clear();
            for (auto const& schema : neos::language::schema::installed())
                if (std::find_if(aExtensions.begin(), aExtensions.end(), [&](neolib::i_string const& e) { return e.to_std_string_view() == schema.first; }) == aExtensions.end())
                    aExtensions.push_back(neolib::string{ schema.first });
        }
        void colours(neolib::i_string const& aExtension, neolib::i_string const& aTheme, neolib::i_vector<neolib::i_string>& aColours) const final
        {
            aColours.clear();
            if (auto const path = schema_path(aExtension.to_std_string()))
                for (auto const& colour : neos::context::highlight_colours(*path, aTheme.to_std_string()))
                    aColours.push_back(neolib::string{ colour });
        }
        bool highlight(neolib::i_string const& aExtension, neolib::i_string const& aScript, neolib::i_vector<span>& aSpans) const final
        {
            aSpans.clear();
            auto const path = schema_path(aExtension.to_std_string());
            if (!path)
                return false;
            std::lock_guard<std::mutex> lock{ iMutex };
            try
            {
                if (!iContext)
                    iContext.emplace(iInput, iOutput, iOutput, neolib::get_service_provider());
                std::string_view const source = aScript.to_std_string_view();
                auto const codePoints = code_points(source);
                // (neos's categories are i_script_highlighter's)
                static_assert(static_cast<std::uint32_t>(neos::context::highlight_category::Variable) == static_cast<std::uint32_t>(category::Variable));
                for (auto const& s : iContext->highlight(*path, source))
                    aSpans.push_back(span{ codePoints[s.first], codePoints[s.last], static_cast<category>(s.category) });
                return true;
            }
            catch (...)
            {
                aSpans.clear();
                return false;
            }
        }
        bool outline(neolib::i_string const& aExtension, neolib::i_string const& aScript, neolib::i_vector<outline_node>& aOutline) const final
        {
            aOutline.clear();
            auto const path = schema_path(aExtension.to_std_string());
            if (!path)
                return false;
            std::lock_guard<std::mutex> lock{ iMutex };
            try
            {
                if (!iContext)
                    iContext.emplace(iInput, iOutput, iOutput, neolib::get_service_provider());
                std::string_view const source = aScript.to_std_string_view();
                auto const codePoints = code_points(source);
                // (neos's kinds are i_script_highlighter's)
                static_assert(static_cast<std::uint32_t>(neos::context::outline_kind::Statement) == static_cast<std::uint32_t>(outline_kind::Statement));
                static_assert(neos::context::outline_node::NoParent == outline_node::NoParent);
                auto const at = [&](std::size_t aByte) { return codePoints[std::min(aByte, source.size())]; };
                for (auto const& n : iContext->outline(*path, source))
                    aOutline.push_back(outline_node{ n.parent, static_cast<outline_kind>(n.kind), at(n.first), at(n.last), at(n.headerLast) });
                return !aOutline.empty();
            }
            catch (...)
            {
                aOutline.clear();
                return false;
            }
        }
    private:
        // (the code point of each byte of a source (and of its end): spans and outlines are of code points, as the text_edit's positions are)
        static std::vector<std::uint32_t> code_points(std::string_view aSource)
        {
            std::vector<std::uint32_t> result(aSource.size() + 1u);
            std::uint32_t codePoint = 0u;
            for (std::size_t i = 0u; i < aSource.size(); ++i)
            {
                if ((static_cast<unsigned char>(aSource[i]) & 0xC0u) != 0x80u)
                    result[i] = codePoint++;
                else
                    result[i] = codePoint - 1u;
            }
            result[aSource.size()] = codePoint;
            return result;
        }
        // (the installed schema for a file extension: neos's (see neos::home))
        std::optional<std::string> const& schema_path(std::string const& aExtension) const
        {
            std::lock_guard<std::mutex> lock{ iPathMutex };
            auto existing = iPaths.find(aExtension);
            if (existing == iPaths.end())
                existing = iPaths.emplace(aExtension, aExtension.empty() ? std::optional<std::string>{} : neos::language::schema::find(aExtension)).first;
            return existing->second;
        }
    private:
        mutable std::mutex iMutex;
        mutable std::mutex iPathMutex;
        mutable std::map<std::string, std::optional<std::string>> iPaths;
        // (the context and its standard streams (unused))
        mutable std::istream iInput{ nullptr };
        mutable std::ostream iOutput{ nullptr };
        mutable std::optional<neos::context> iContext;
    };

    neos_element_library::neos_element_library(neolib::i_application& aApplication, std::string const& aPluginPath) :
        iApplication{ aApplication },
        iPluginPath{ aPluginPath },
        iClientManager{ aApplication },
        iElementsOrdered
        {
        },
        iElements{ elements_ordered().begin(), elements_ordered().end() },
        iSeparator{},
        iNewScript{ "New Script..."_t },
        iStartDebugging{ "Start Debugging"_t },
        iStartWithoutDebugging{ "Start Without Debugging"_t },
        iStepInto{ "Step Into"_t },
        iStepOver{ "Step Over"_t },
        iToggleBreakpoint{ "Toggle Breakpoint"_t },
        iNewBreakpoint{ "New Breakpoint"_t },
        iDeleteAllBreakpoints{ "Delete All Breakpoints"_t },
        iDisableAllBreakpoints{ "Disable All Breakpoints"_t },
        iImportScriptingLanguage{ "Import Scripting Language..."_t },
        iEditScriptingLanguage{ "Edit Scripting Language..."_t },
        iScriptMenu{ "Script"_t }
    {
        iSeparator.set_group(library_id());
        iNewScript.set_group(library_id());
        iStartDebugging.set_group(library_id()).set_shortcut("F5");
        iStartWithoutDebugging.set_group(library_id()).set_shortcut("Ctrl+F5");
        iStepInto.set_group(library_id()).set_shortcut("F11");
        iStepOver.set_group(library_id()).set_shortcut("F10");
        iToggleBreakpoint.set_group(library_id()).set_shortcut("F9");
        iNewBreakpoint.set_group(library_id());
        iDeleteAllBreakpoints.set_group(library_id());
        iDisableAllBreakpoints.set_group(library_id());
        iImportScriptingLanguage.set_group(library_id());
        iEditScriptingLanguage.set_group(library_id());

        iScriptMenu.set_group(library_id());

        // (neos's concept libraries, compiler plugins, runtime plugins and languages: the installed neos's, not this program's folder's)
        neos::set_home(neos::installed_home());

        // (the syntax highlighting of scripts: a service, for the script editors of other libraries)
        iScriptHighlighter = std::make_unique<neos_script_highlighter>();
        neolib::services::register_service<i_script_highlighter>(*iScriptHighlighter);
        iScriptMenu.add_action(iNewScript);
        iScriptMenu.add_action(iSeparator);
        iScriptMenu.add_action(iStartDebugging);
        iScriptMenu.add_action(iStartWithoutDebugging);
        iScriptMenu.add_action(iSeparator);
        iScriptMenu.add_action(iStepInto);
        iScriptMenu.add_action(iStepOver);
        iScriptMenu.add_action(iSeparator);
        iScriptMenu.add_action(iToggleBreakpoint);
        iScriptMenu.add_action(iNewBreakpoint);
        iScriptMenu.add_action(iDeleteAllBreakpoints);
        iScriptMenu.add_action(iDisableAllBreakpoints);
        iScriptMenu.add_action(iSeparator);
        iScriptMenu.add_action(iImportScriptingLanguage);
        iScriptMenu.add_action(iEditScriptingLanguage);

        iClientManager->start_console_client_session(
            [&](i_terminal& aTerminal, i_string const& aCommand, i_ref_ptr<i_console_client>& aClient)
            {
                auto bits = neolib::tokens(aCommand.to_std_string(), " "s);
                if (bits.size() >= 1)
                {
                    if (bits[0] == "neos")
                    {
                        // ("neos <program>": a session that only runs programs (see neos_session))
                        auto const command = aCommand.to_std_string();
                        auto const program = command.find_first_not_of(' ', command.find("neos") + 4u);
                        aClient = make_ref<neos_session>(neolib::get_service_provider(), 
                            program == std::string::npos ? std::string{} : command.substr(program));
                    }
                }
            });
    }

    neos_element_library::~neos_element_library()
    {
        neolib::services::unregister_service<i_script_highlighter>(*iScriptHighlighter);
    }

    neolib::i_application& neos_element_library::application() const
    {
        return iApplication;
    }

    void neos_element_library::ide_ready(i_ide& aIde)
    {
        aIde.add_sub_menu(id::MenuBar, iScriptMenu);
    }

    const neos_element_library::elements_t& neos_element_library::elements() const
    {
        return iElements;
    }

    const neos_element_library::elements_ordered_t& neos_element_library::elements_ordered() const
    {
        return iElementsOrdered;
    }

    void neos_element_library::create_element(i_project& aProject, const neolib::i_string& aElementType, const neolib::i_string& aElementId, neolib::i_ref_ptr<i_element>& aResult)
    {
        static const std::map<string, std::function<i_element* (i_project& aProject, const neolib::i_string&)>> sFactoryMethods =
        {
            #define MAKE_ROOT_ELEMENT_FACTORY_FUNCTION(Type) { #Type, [this](i_project& aProject, const neolib::i_string& aElementId) -> i_element* { return new element<Type>{ *this, aProject, string{ #Type }, aElementId }; } },
            #define MAKE_NAMED_ROOT_ELEMENT_FACTORY_FUNCTION(Name, Type) { #Name, [this](i_project& aProject, const neolib::i_string& aElementId) -> i_element* { return new element<Type>{ *this, aProject, string{ #Name }, aElementId }; } },
            MAKE_NAMED_ROOT_ELEMENT_FACTORY_FUNCTION(project, i_project)
        };
        auto method = sFactoryMethods.find(aElementType);
        if (method != sFactoryMethods.end())
        {
            aResult.reset((method->second)(aProject, aElementId));
            return;
        }
        throw unknown_element_type();
    }

    void neos_element_library::create_element(i_element& aParent, const neolib::i_string& aElementType, const neolib::i_string& aElementId, neolib::i_ref_ptr<i_element>& aResult)
    {
        static const std::map<string, std::function<i_element*(i_element&, const neolib::i_string&, const neolib::i_string&)>> sFactoryMethods =
        {
            #define MAKE_ELEMENT_FACTORY_FUNCTION(Type) { #Type, [this](i_element& aParent, const neolib::i_string& aElementType, const neolib::i_string& aElementId) -> i_element* { return new element<Type>{ *this, aParent, string{ #Type }, aElementId, element_group(aElementType) }; } },
            #define MAKE_NAMED_ELEMENT_FACTORY_FUNCTION(Name, Type) { #Name, [this](i_element& aParent, const neolib::i_string& aElementType, const neolib::i_string& aElementId) -> i_element* { return new element<Type>{ *this, aParent, string{ #Name }, aElementId, element_group(aElementType) }; } },
        };
        auto method = sFactoryMethods.find(aElementType);
        if (method != sFactoryMethods.end())
        {
            aResult.reset((method->second)(aParent, aElementType, aElementId));
            return;
        }
        throw unknown_element_type();
    }

    element_group neos_element_library::element_group(const neolib::i_string& aElementType) const
    {
        static const std::map<string, DesignStudio::element_group> sElementGroups =
        {
        };
        auto group = sElementGroups.find(aElementType);
        if (group != sElementGroups.end())
        {
            return group->second;
        }
        throw unknown_element_type();
    }

    i_texture const& neos_element_library::element_icon(const neolib::i_string& aElementType) const
    {
        auto& icons = iIcons[service<i_app>().current_style().palette().color(color_role::Text)];
        static std::map<string, std::function<void(texture&)>> sIconResources =
        {
        };
        auto existing = sIconResources.find(aElementType);
        if (existing != sIconResources.end())
        {
            if (icons.find(aElementType) == icons.end())
                existing->second(icons[aElementType]);
            return icons[aElementType];
        }
        if (icons.find("default") == icons.end())
            icons["default"] = texture{ size{32.0, 32.0} };
        return icons["default"];
    }
}
