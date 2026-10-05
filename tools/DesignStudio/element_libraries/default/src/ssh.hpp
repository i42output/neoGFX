// ssh.hpp
/*
  neoGFX Design Studio
  Copyright(C) 2026 Leigh Johnston

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

#pragma once

#include <neogfx/neogfx.hpp>

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>

#include <neolib/task/timer.hpp>

#include <neogfx/tools/DesignStudio/console_client.hpp>

#include <neolib/secure/secure_string.hpp>

#include <libssh/libssh.h>
#include <libssh/callbacks.h>

namespace neogfx::DesignStudio
{
    // All libssh calls are made on a worker thread; the GUI thread only exchanges data with
    // the worker through shared_state (under a mutex held only for brief copies) and polls
    // for worker results with a timer, so the GUI thread never blocks.
    class ssh : public console_client
    {
    public:
        define_event(ConnectionFailure, connection_failure, std::string const&)
        define_event(Disconnected, disconnected)
    private:
        struct shared_state
        {
            std::mutex mutex;
            std::condition_variable wake;
            std::atomic<bool> cancelled = false;
            // GUI thread -> worker thread
            neolib::secure_string input;
            std::optional<std::pair<std::uint16_t, std::uint16_t>> windowSize;
            std::optional<neolib::secure_string> answer;
            // worker thread -> GUI thread
            std::string output;
            std::optional<bool> lineRequest; // value is echo flag
            bool open = false;
            std::optional<std::string> failure;
            bool disconnected = false;
        };
        struct connection_parameters
        {
            std::string host;
            std::optional<std::string> user;
            std::optional<unsigned int> port;
            std::optional<std::string> identity;
            std::uint16_t windowWidth;
            std::uint16_t windowHeight;
        };
        class session
        {
        private:
            enum class state
            {
                Connecting,
                VerifyHost,
                ConfirmHost,
                AuthNone,
                AuthPublicKey,
                AuthKeyboardInteractive,
                KeyboardInteractiveInput,
                PasswordInput,
                AuthPassword,
                OpenChannel,
                RequestPty,
                RequestShell,
                Open,
                Finished
            };
            static constexpr int MaxAuthAttempts = 3;
        public:
            session(std::shared_ptr<shared_state> aShared, connection_parameters const& aParameters) :
                iShared{ std::move(aShared) },
                iParameters{ aParameters }
            {
            }
        public:
            void run()
            {
                ssh_init();
                // libssh logs to stderr by default; route its logging through the neoGFX logger
                ssh_set_log_callback(&session::log_callback);
                if (setup())
                {
                    while (!iShared->cancelled && iState != state::Finished)
                    {
                        exchange();
                        while (!iShared->cancelled && step());
                        if (iShared->cancelled || iState == state::Finished)
                            break;
                        std::unique_lock<std::mutex> lock{ iShared->mutex };
                        iShared->wake.wait_for(lock, std::chrono::milliseconds{ 10 }, [&]()
                        {
                            return iShared->cancelled || !iShared->input.empty() || iShared->answer || iShared->windowSize;
                        });
                    }
                }
                iAnswer.reset();
                iPassword.clear();
                iWriteBuffer.clear();
                if (iChannel)
                    ssh_channel_free(iChannel);
                if (iSession)
                {
                    ssh_disconnect(iSession);
                    ssh_free(iSession);
                }
                ssh_finalize();
            }
        private:
            bool setup()
            {
                iSession = ssh_new();
                if (!iSession)
                    return fail("Failed to create SSH session.");
                if (ssh_options_set(iSession, SSH_OPTIONS_HOST, iParameters.host.c_str()) != SSH_OK)
                    return fail(error());
                ssh_options_parse_config(iSession, nullptr);
                if (iParameters.user && ssh_options_set(iSession, SSH_OPTIONS_USER, iParameters.user->c_str()) != SSH_OK)
                    return fail(error());
                if (iParameters.port && ssh_options_set(iSession, SSH_OPTIONS_PORT, &*iParameters.port) != SSH_OK)
                    return fail(error());
                if (iParameters.identity && ssh_options_set(iSession, SSH_OPTIONS_IDENTITY, iParameters.identity->c_str()) != SSH_OK)
                    return fail(error());
                // libssh (via OpenSSL) prompts on the console for private key passphrases unless an
                // auth callback is provided; prompt through the neoGFX terminal instead
                ssh_callbacks_init(&iCallbacks);
                iCallbacks.userdata = this;
                iCallbacks.auth_function = &session::auth_callback;
                if (ssh_set_callbacks(iSession, &iCallbacks) != SSH_OK)
                    return fail(error());
                // non-blocking so that cancellation is noticed promptly
                ssh_set_blocking(iSession, 0);
                return true;
            }
            void exchange()
            {
                std::optional<std::pair<std::uint16_t, std::uint16_t>> windowSize;
                {
                    std::lock_guard<std::mutex> lock{ iShared->mutex };
                    windowSize = std::exchange(iShared->windowSize, std::nullopt);
                    iWriteBuffer += iShared->input;
                    iShared->input.clear();
                    if (iShared->answer)
                    {
                        iAnswer = std::move(*iShared->answer);
                        iShared->answer.reset();
                    }
                }
                if (windowSize)
                {
                    iParameters.windowWidth = windowSize->first;
                    iParameters.windowHeight = windowSize->second;
                    if (iState == state::Open)
                        ssh_channel_change_pty_size(iChannel, iParameters.windowWidth, iParameters.windowHeight);
                }
            }
            // returns true if another step should be taken immediately
            bool step()
            {
                switch (iState)
                {
                case state::Connecting:
                    {
                        auto const rc = ssh_connect(iSession);
                        if (rc == SSH_AGAIN)
                            return false;
                        if (rc != SSH_OK)
                            return fail(error());
                        iState = state::VerifyHost;
                    }
                    return true;
                case state::VerifyHost:
                    switch (ssh_session_is_known_server(iSession))
                    {
                    case SSH_KNOWN_HOSTS_OK:
                        iState = state::AuthNone;
                        return true;
                    case SSH_KNOWN_HOSTS_CHANGED:
                        return fail("WARNING: REMOTE HOST IDENTIFICATION HAS CHANGED!\r\n" +
                            fingerprint() + ".\r\nHost key for '" + iParameters.host + "' does not match the one in known_hosts.\r\nHost key verification failed.");
                    case SSH_KNOWN_HOSTS_OTHER:
                        return fail("The host key for '" + iParameters.host + "' was not found but a key of another type exists in known_hosts.\r\nHost key verification failed.");
                    case SSH_KNOWN_HOSTS_NOT_FOUND:
                    case SSH_KNOWN_HOSTS_UNKNOWN:
                        output("The authenticity of host '" + iParameters.host + "' can't be established.\r\n" + fingerprint() +
                            ".\r\nAre you sure you want to continue connecting (yes/no)? ");
                        request_line(state::ConfirmHost, true);
                        return false;
                    case SSH_KNOWN_HOSTS_ERROR:
                    default:
                        return fail(error());
                    }
                case state::ConfirmHost:
                    if (!iAnswer)
                        return false;
                    if (*iAnswer == "yes")
                    {
                        if (ssh_session_update_known_hosts(iSession) == SSH_OK)
                            output("Warning: Permanently added '" + iParameters.host + "' to the list of known hosts.\r\n");
                        else
                            output("Warning: failed to add '" + iParameters.host + "' to the list of known hosts: " + error() + "\r\n");
                        iAnswer.reset();
                        iState = state::AuthNone;
                        return true;
                    }
                    if (*iAnswer == "no")
                        return fail("Host key verification failed.");
                    iAnswer.reset();
                    output("Please type 'yes' or 'no': ");
                    request_line(state::ConfirmHost, true);
                    return false;
                case state::AuthNone:
                    {
                        auto const rc = ssh_userauth_none(iSession, nullptr);
                        if (rc == SSH_AUTH_AGAIN)
                            return false;
                        if (rc == SSH_AUTH_SUCCESS)
                        {
                            iState = state::OpenChannel;
                            return true;
                        }
                        if (rc == SSH_AUTH_ERROR)
                            return fail(error());
                        iAuthMethods = ssh_userauth_list(iSession, nullptr);
                    }
                    return next_auth_method();
                case state::AuthPublicKey:
                    {
                        auto const rc = ssh_userauth_publickey_auto(iSession, nullptr, nullptr);
                        if (rc == SSH_AUTH_AGAIN)
                            return false;
                        return auth_result(rc);
                    }
                case state::AuthKeyboardInteractive:
                    {
                        auto const rc = ssh_userauth_kbdint(iSession, nullptr, nullptr);
                        if (rc == SSH_AUTH_AGAIN)
                            return false;
                        if (rc == SSH_AUTH_INFO)
                        {
                            iPromptCount = ssh_userauth_kbdint_getnprompts(iSession);
                            iPromptIndex = 0;
                            if (iPromptCount <= 0)
                                return true; // empty info request; answer it
                            if (auto const name = ssh_userauth_kbdint_getname(iSession); name && *name)
                                output(std::string{ name } + "\r\n");
                            if (auto const instruction = ssh_userauth_kbdint_getinstruction(iSession); instruction && *instruction)
                                output(std::string{ instruction } + "\r\n");
                            prompt_keyboard_interactive();
                            return false;
                        }
                        return auth_result(rc);
                    }
                case state::KeyboardInteractiveInput:
                    if (!iAnswer)
                        return false;
                    {
                        auto const rc = ssh_userauth_kbdint_setanswer(iSession, static_cast<unsigned int>(iPromptIndex), iAnswer->c_str());
                        iAnswer.reset();
                        if (rc < 0)
                            return fail(error());
                    }
                    if (++iPromptIndex < iPromptCount)
                    {
                        prompt_keyboard_interactive();
                        return false;
                    }
                    iState = state::AuthKeyboardInteractive;
                    return true;
                case state::PasswordInput:
                    if (!iAnswer)
                        return false;
                    iPassword.clear();
                    iPassword = std::move(*iAnswer);
                    iAnswer.reset();
                    iState = state::AuthPassword;
                    return true;
                case state::AuthPassword:
                    {
                        auto const rc = ssh_userauth_password(iSession, nullptr, iPassword.c_str());
                        if (rc == SSH_AUTH_AGAIN)
                            return false;
                        iPassword.clear();
                        return auth_result(rc);
                    }
                case state::OpenChannel:
                    {
                        if (!iChannel)
                            iChannel = ssh_channel_new(iSession);
                        if (!iChannel)
                            return fail(error());
                        auto const rc = ssh_channel_open_session(iChannel);
                        if (rc == SSH_AGAIN)
                            return false;
                        if (rc != SSH_OK)
                            return fail(error());
                        iState = state::RequestPty;
                    }
                    return true;
                case state::RequestPty:
                    {
                        auto const rc = ssh_channel_request_pty_size(iChannel, "xterm", iParameters.windowWidth, iParameters.windowHeight);
                        if (rc == SSH_AGAIN)
                            return false;
                        if (rc != SSH_OK)
                            return fail(error());
                        iState = state::RequestShell;
                    }
                    return true;
                case state::RequestShell:
                    {
                        auto const rc = ssh_channel_request_shell(iChannel);
                        if (rc == SSH_AGAIN)
                            return false;
                        if (rc != SSH_OK)
                            return fail(error());
                        iState = state::Open;
                        std::lock_guard<std::mutex> lock{ iShared->mutex };
                        iShared->open = true;
                    }
                    return true;
                case state::Open:
                    {
                        if (!iWriteBuffer.empty())
                        {
                            auto const rc = ssh_channel_write(iChannel, iWriteBuffer.data(), static_cast<std::uint32_t>(iWriteBuffer.size()));
                            if (rc == SSH_ERROR)
                                return fail(error());
                            if (rc > 0)
                                iWriteBuffer.erase(0, static_cast<std::size_t>(rc));
                        }
                        char buffer[16384];
                        std::string received;
                        for (int stream = 0; stream <= 1; ++stream)
                            while (received.size() < 0x100000u)
                            {
                                auto const count = ssh_channel_read_nonblocking(iChannel, buffer, static_cast<std::uint32_t>(sizeof(buffer)), stream);
                                if (count > 0)
                                    received.append(buffer, static_cast<std::size_t>(count));
                                else if (count == SSH_ERROR)
                                {
                                    output(received);
                                    return fail(error());
                                }
                                else
                                    break;
                            }
                        output(received);
                        if (ssh_channel_is_eof(iChannel) || ssh_channel_is_closed(iChannel))
                        {
                            output("\r\nConnection to " + iParameters.host + " closed.\r\n");
                            iState = state::Finished;
                            std::lock_guard<std::mutex> lock{ iShared->mutex };
                            iShared->open = false;
                            iShared->disconnected = true;
                        }
                    }
                    return false;
                case state::Finished:
                default:
                    return false;
                }
            }
            bool next_auth_method()
            {
                if ((iAuthMethods & SSH_AUTH_METHOD_PUBLICKEY) && !iTriedPublicKey)
                {
                    iTriedPublicKey = true;
                    iState = state::AuthPublicKey;
                    return true;
                }
                if ((iAuthMethods & SSH_AUTH_METHOD_INTERACTIVE) && iKeyboardInteractiveAttempts < MaxAuthAttempts)
                {
                    ++iKeyboardInteractiveAttempts;
                    iState = state::AuthKeyboardInteractive;
                    return true;
                }
                if ((iAuthMethods & SSH_AUTH_METHOD_PASSWORD) && iPasswordAttempts < MaxAuthAttempts)
                {
                    ++iPasswordAttempts;
                    output(username() + "@" + iParameters.host + "'s password: ");
                    request_line(state::PasswordInput, false);
                    return false;
                }
                return fail("Permission denied.");
            }
            bool auth_result(int aResult)
            {
                switch (aResult)
                {
                case SSH_AUTH_SUCCESS:
                    iState = state::OpenChannel;
                    return true;
                case SSH_AUTH_PARTIAL:
                    iAuthMethods = ssh_userauth_list(iSession, nullptr);
                    return next_auth_method();
                case SSH_AUTH_DENIED:
                    if (iState == state::AuthPassword || iState == state::AuthKeyboardInteractive)
                        output("Permission denied, please try again.\r\n");
                    return next_auth_method();
                case SSH_AUTH_ERROR:
                default:
                    return fail(error());
                }
            }
            void prompt_keyboard_interactive()
            {
                char echo = 0;
                auto const prompt = ssh_userauth_kbdint_getprompt(iSession, static_cast<unsigned int>(iPromptIndex), &echo);
                output(prompt ? std::string{ prompt } : std::string{ "Response: " });
                request_line(state::KeyboardInteractiveInput, echo != 0);
            }
            void request_line(state aState, bool aEcho)
            {
                iAnswer.reset();
                iState = aState;
                std::lock_guard<std::mutex> lock{ iShared->mutex };
                iShared->lineRequest = aEcho;
            }
            void output(std::string const& aText)
            {
                if (aText.empty())
                    return;
                std::lock_guard<std::mutex> lock{ iShared->mutex };
                iShared->output += aText;
            }
            bool fail(std::string const& aError)
            {
                iState = state::Finished;
                std::lock_guard<std::mutex> lock{ iShared->mutex };
                iShared->open = false;
                iShared->failure = aError;
                return false;
            }
            std::string error() const
            {
                if (iSession)
                    if (auto const message = ssh_get_error(iSession); message && *message)
                        return message;
                return "SSH error.";
            }
            std::string username() const
            {
                std::string result;
                char* user = nullptr;
                if (ssh_options_get(iSession, SSH_OPTIONS_USER, &user) == SSH_OK && user)
                {
                    result = user;
                    ssh_string_free_char(user);
                }
                return result;
            }
            // called on this (worker) thread by libssh when a private key passphrase is needed
            static int auth_callback(const char* aPrompt, char* aBuffer, std::size_t aLength, int aEcho, int, void* aUserData)
            {
                return static_cast<session*>(aUserData)->passphrase(aPrompt, aBuffer, aLength, aEcho != 0);
            }
            int passphrase(const char* aPrompt, char* aBuffer, std::size_t aLength, bool aEcho)
            {
                std::string prompt = aPrompt && *aPrompt ? aPrompt : "Enter passphrase:";
                if (prompt.back() != ' ')
                    prompt += ' ';
                output(prompt);
                auto answer = wait_for_line(aEcho);
                if (!answer || answer->empty() || answer->size() >= aLength)
                {
                    answer.reset();
                    return -1;
                }
                std::copy(answer->begin(), answer->end(), aBuffer);
                aBuffer[answer->size()] = '\0';
                answer.reset();
                return 0;
            }
            // blocks this (worker) thread until the GUI thread supplies a line or the session is cancelled
            std::optional<neolib::secure_string> wait_for_line(bool aEcho)
            {
                std::unique_lock<std::mutex> lock{ iShared->mutex };
                iShared->lineRequest = aEcho;
                iShared->wake.wait(lock, [&]() { return iShared->cancelled || iShared->answer.has_value(); });
                if (iShared->cancelled)
                    return std::nullopt;
                std::optional<neolib::secure_string> result = std::move(iShared->answer);
                iShared->answer.reset();
                return result;
            }
            static void log_callback(int aPriority, const char* aFunction, const char* aMessage, void*)
            {
                auto const severity =
                    aPriority <= SSH_LOG_WARN ? neolib::logger::severity::Warning :
                    aPriority == SSH_LOG_INFO ? neolib::logger::severity::Info :
                    aPriority == SSH_LOG_DEBUG ? neolib::logger::severity::Debug :
                        neolib::logger::severity::Trace;
                service<debug::logger>() << severity << "libssh: " << (aFunction ? aFunction : "") << ": " << (aMessage ? aMessage : "") << std::endl;
            }
            std::string fingerprint() const
            {
                std::string result = "Key fingerprint unavailable";
                ssh_key key = nullptr;
                if (ssh_get_server_publickey(iSession, &key) == SSH_OK)
                {
                    unsigned char* hash = nullptr;
                    std::size_t hashLength = 0u;
                    if (ssh_get_publickey_hash(key, SSH_PUBLICKEY_HASH_SHA256, &hash, &hashLength) == SSH_OK)
                    {
                        if (char* hexa = ssh_get_fingerprint_hash(SSH_PUBLICKEY_HASH_SHA256, hash, hashLength))
                        {
                            auto const keyType = ssh_key_type_to_char(ssh_key_type(key));
                            result = std::string{ keyType ? keyType : "Host" } + " key fingerprint is " + hexa;
                            ssh_string_free_char(hexa);
                        }
                        ssh_clean_pubkey_hash(&hash);
                    }
                    ssh_key_free(key);
                }
                return result;
            }
        private:
            std::shared_ptr<shared_state> iShared;
            connection_parameters iParameters;
            ssh_session iSession = nullptr;
            ssh_channel iChannel = nullptr;
            state iState = state::Connecting;
            int iAuthMethods = 0;
            bool iTriedPublicKey = false;
            int iKeyboardInteractiveAttempts = 0;
            int iPasswordAttempts = 0;
            int iPromptCount = 0;
            int iPromptIndex = 0;
            std::optional<neolib::secure_string> iAnswer;
            neolib::secure_string iPassword;
            neolib::secure_string iWriteBuffer;
            ssh_callbacks_struct iCallbacks = {};
        };
    public:
        ssh()
        {
        }
        ~ssh()
        {
            iPoller.reset();
            if (iShared)
            {
                iShared->cancelled = true;
                iShared->wake.notify_one();
            }
            // the worker is never joined (it may be inside a blocking host name lookup); it owns
            // everything it touches (via shared_state) and exits as soon as it can.
            iLine.clear();
        }
    public:
        void start() final
        {
        }
        void resize_window(std::uint16_t aWidth, std::uint16_t aHeight) final
        {
            if (aWidth == 0u || aHeight == 0u)
                return;
            iWindowWidth = aWidth;
            iWindowHeight = aHeight;
            if (iShared)
            {
                {
                    std::lock_guard<std::mutex> lock{ iShared->mutex };
                    iShared->windowSize.emplace(iWindowWidth, iWindowHeight);
                }
                iShared->wake.notify_one();
            }
        }
        void input(std::string const& aText) final
        {
            if (!iShared || iFinished)
                return;
            if (iOpen)
            {
                // terminal sends "\n" after "\r" for Enter; a pty only wants the "\r"
                if (iLastInputWasCR && aText == "\n")
                {
                    iLastInputWasCR = false;
                    return;
                }
                iLastInputWasCR = !aText.empty() && aText.back() == '\r';
                {
                    std::lock_guard<std::mutex> lock{ iShared->mutex };
                    iShared->input.append(aText.data(), aText.size());
                }
                iShared->wake.notify_one();
                return;
            }
            if (aText == "\x03")
            {
                iShared->cancelled = true;
                iShared->wake.notify_one();
                iPendingFailure = "Connection aborted.";
                return;
            }
            if (!iLineInput || aText.empty() || aText[0] == '\x1B')
                return;
            for (char ch : aText)
            {
                if (ch == '\r')
                {
                    Output("\r\n");
                    iLineInput = false;
                    {
                        std::lock_guard<std::mutex> lock{ iShared->mutex };
                        iShared->answer = std::move(iLine);
                    }
                    iLine.clear();
                    iShared->wake.notify_one();
                    return;
                }
                else if (ch == '\x7F' || ch == '\b')
                {
                    if (!iLine.empty())
                    {
                        iLine.pop_back();
                        if (iLineEcho)
                            Output("\b \b");
                    }
                }
                else if (static_cast<unsigned char>(ch) >= 0x20u)
                {
                    iLine.push_back(ch);
                    if (iLineEcho)
                        Output(std::string(1, ch));
                }
            }
        }
    public:
        // usage: ssh [-p port] [-l login] [-i identity_file] [user@]host[:port]
        void connect(std::vector<std::string> const& aArgs)
        {
            start_polling();

            static std::string const usage = "usage: ssh [-p port] [-l login] [-i identity_file] [user@]host[:port]";

            connection_parameters parameters{ {}, {}, {}, {}, iWindowWidth, iWindowHeight };
            std::optional<std::string> port;
            std::string target;
            for (std::size_t i = 0; i < aArgs.size(); ++i)
            {
                auto const& arg = aArgs[i];
                if (arg.empty())
                    continue;
                if ((arg == "-p" || arg == "-l" || arg == "-i") && i + 1 < aArgs.size())
                {
                    auto const& value = aArgs[++i];
                    (arg == "-p" ? port : arg == "-l" ? parameters.user : parameters.identity) = value;
                }
                else if (arg[0] == '-' || !target.empty())
                {
                    iPendingFailure = usage;
                    return;
                }
                else
                    target = arg;
            }
            if (target.empty())
            {
                iPendingFailure = usage;
                return;
            }

            auto const at = target.rfind('@');
            if (at != std::string::npos)
            {
                if (!parameters.user)
                    parameters.user = target.substr(0, at);
                target = target.substr(at + 1);
            }
            std::optional<std::string> targetPort;
            if (!target.empty() && target[0] == '[')
            {
                auto const close = target.find(']');
                if (close == std::string::npos)
                {
                    iPendingFailure = usage;
                    return;
                }
                parameters.host = target.substr(1, close - 1);
                if (close + 1 < target.size() && target[close + 1] == ':')
                    targetPort = target.substr(close + 2);
            }
            else if (std::count(target.begin(), target.end(), ':') == 1)
            {
                auto const colon = target.find(':');
                parameters.host = target.substr(0, colon);
                targetPort = target.substr(colon + 1);
            }
            else
                parameters.host = target;
            if (!port)
                port = targetPort;

            if (port)
            {
                try
                {
                    auto const value = std::stoul(*port);
                    if (value == 0ul || value > 65535ul)
                        throw std::out_of_range{ "port" };
                    parameters.port = static_cast<unsigned int>(value);
                }
                catch (...)
                {
                    iPendingFailure = "Bad port '" + *port + "'.";
                    return;
                }
            }

            iShared = std::make_shared<shared_state>();
            // The worker thread is detached and never joined (it may be inside a blocking host name
            // lookup); it owns everything it touches (via shared_state) and exits as soon as it can.
            // On Windows the CRT's _beginthreadex (used by std::thread) holds a reference on the module
            // containing the thread procedure (std::thread's invoker, instantiated in this module) until
            // the thread exits, so this module cannot be unloaded while the worker is still running.
            try
            {
                std::thread{ [shared = iShared, parameters]()
                {
                    try
                    {
                        session{ shared, parameters }.run();
                    }
                    catch (...)
                    {
                        std::lock_guard<std::mutex> lock{ shared->mutex };
                        shared->open = false;
                        shared->failure = "SSH worker thread failed.";
                    }
                } }.detach();
            }
            catch (std::exception const& e)
            {
                iPendingFailure = std::string{ "Failed to start SSH thread: " } + e.what();
            }
        }
    private:
        void start_polling()
        {
            iPoller.emplace(neolib::service<neolib::i_async_task>(), [this](neolib::callback_timer& aTimer)
            {
                destroyed_flag destroyed{ *this };
                poll();
                if (!destroyed && !iFinished)
                    aTimer.again();
            }, std::chrono::milliseconds{ 10 });
        }
        // GUI thread: collect results from the worker; never waits on the worker.
        void poll()
        {
            std::string output;
            std::optional<bool> lineRequest;
            std::optional<std::string> failure = std::move(iPendingFailure);
            iPendingFailure = std::nullopt;
            bool disconnected = false;
            if (iShared)
            {
                std::lock_guard<std::mutex> lock{ iShared->mutex };
                output.swap(iShared->output);
                lineRequest = std::exchange(iShared->lineRequest, std::nullopt);
                iOpen = iShared->open;
                if (!failure)
                    failure = std::exchange(iShared->failure, std::nullopt);
                disconnected = std::exchange(iShared->disconnected, false);
            }
            if (!output.empty())
                Output(output);
            if (lineRequest)
            {
                iLine.clear();
                iLineEcho = *lineRequest;
                iLineInput = true;
            }
            if (failure)
            {
                iFinished = true;
                ConnectionFailure(*failure);
                return; // *this may no longer exist
            }
            if (disconnected)
            {
                iFinished = true;
                Disconnected();
                return; // *this may no longer exist
            }
        }
    private:
        std::shared_ptr<shared_state> iShared;
        std::optional<neolib::callback_timer> iPoller;
        std::optional<std::string> iPendingFailure;
        bool iFinished = false;
        bool iOpen = false;
        std::uint16_t iWindowWidth = 80;
        std::uint16_t iWindowHeight = 25;
        neolib::secure_string iLine;
        bool iLineEcho = true;
        bool iLineInput = false;
        bool iLastInputWasCR = false;
    };
}
