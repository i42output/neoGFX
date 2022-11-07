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
#include <boost/range/iterator_range.hpp>
#include <neolib/core/string_ci.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gfx/utility.hpp>
#include <neogfx/tools/DesignStudio/i_project.hpp>
#include <neogfx/tools/DesignStudio/element.hpp>
#include <neogfx/tools/DesignStudio/console_client.hpp>
#include <neos/context.hpp>
#include "library.hpp"

namespace neogfx::DesignStudio
{
    class neos_session : public console
    {
    public:
        neos_session(neolib::i_service_provider& aServiceProvider) :
            console{ "neos" },
            iContext{ cout, aServiceProvider }
        {
            command([&](std::string const& aCommand)
            {
                process_command(aCommand);
            });
        }
    private:
        bool process_command(std::string const& aCommand)
        {
            auto output_compilation_time = [&]()
            {
                cout << "Compilation time: "s <<
                    std::chrono::duration_cast<std::chrono::microseconds>(iContext.compiler().end_time() - 
                        iContext.compiler().start_time()).count() / 1000.0 << "ms" << std::endl;
            };

            auto previousCurrentPath = std::filesystem::current_path();
            std::filesystem::current_path(std::filesystem::path{ std::getenv("DevDirNeos") } / "languages");
            std::vector<std::pair<std::string::const_iterator, std::string::const_iterator>> words;
            const std::string delimeters{ " " };
            neolib::tokens(aCommand.cbegin(), aCommand.cend(), delimeters.cbegin(), delimeters.cend(), words);
            if (words.empty())
                return true;
            const std::string command{ words[0].first, words[0].second };
            if (command.empty())
                return true;
            std::string parameters{ words.size() > 1 ? words[1].first : aCommand.cend(), aCommand.cend() };
            try
            {
                if (iInteractive)
                    iContext.evaluate(aCommand);
                else if (command == "?" || command == "h" || command == "help")
                {
                    cout << "h(elp)\n"
                        << "s(chema) <path to language schema>       Load language schema\n"
                        << "l(oad) <path to program>                 Load program\n"
                        << "list                                     List program\n"
                        << "c(ompile)                                Compile program\n"
                        << "r(un)                                    Run program\n"
                        << "![<expression>]                          Evaluate expression (enter interactive mode if expression omitted)\n"
                        << ":<input>                                 Input (as stdin)\n"
                        << "q(uit)                                   Quit neos\n"
                        << "lc                                       List loaded concept libraries\n"
                        << "t(race) <0|1|2|3|4|5> [<filter>]         Compiler trace\n"
                        << "m(etrics)                                Display metrics for running programs\n"
                        << std::flush;
                }
                else if (command == "s" || command == "schema")
                {
                    iContext.load_schema(parameters);
                    cout << "Language: " << iContext.schema().meta().description + "\nVersion: " + iContext.schema().meta().version << std::endl;
                    if (!iContext.schema().meta().copyright.empty())
                        cout << iContext.schema().meta().copyright << std::endl;
                }
                else if (command == "l" || command == "load")
                {
                    iContext.load_program(parameters);
                    cout << "Program: " << parameters << std::endl;
                }
                else if (command == "c" || command == "compile")
                {
                    iContext.compile_program();
                    output_compilation_time();
                }
                else if (command == "list")
                {
                    for (auto const& tu : iContext.program().translationUnits)
                        for (auto const& f : tu.fragments)
                        {
                            if (f.source_file_path() != std::nullopt)
                                cout << "File '" << *f.source_file_path() << "':" << std::endl;
                            cout << f.source() << std::endl;
                        }
                }
                else if (command == "r" || command == "run")
                    iContext.run();
                else if (command[0] == '!')
                {
                    if (!iContext.schema_loaded())
                        throw std::runtime_error("no schema loaded");
                    std::string expression(std::next(words[0].first), aCommand.cend());
                    if (!expression.empty())
                    {
                        iContext.evaluate(expression);
                        output_compilation_time();
                    }
                    else
                        iInteractive = true;
                }
                else if (command == "lc")
                {
                    auto print_concept_library = [&](auto const& self, const neos::language::i_concept_library& aConceptLibrary, uint32_t aDepth = 0) -> void
                    {
                        cout << std::string(aDepth * 2, ' ') << "[" << aConceptLibrary.name() << "]";
                        if (aDepth == 0)
                            cout << " (" << aConceptLibrary.uri() << ")";
                        cout << std::endl;
                        for (auto const& conceptLibrary : aConceptLibrary.sublibraries())
                            self(self, *conceptLibrary.second(), aDepth + 1);
                    };
                    for (auto const& conceptLibrary : iContext.concept_libraries())
                        if (conceptLibrary.second()->depth() == 0)
                            print_concept_library(print_concept_library, *conceptLibrary.second());
                }
                else if (command == "t" || command == "trace")
                {
                    if (words.size() >= 2)
                        iContext.compiler().set_trace(
                            boost::lexical_cast<uint32_t>(std::string{ words[1].first, words[1].second }),
                            words.size() > 2 ? std::string{ words[2].first, words[2].second } : std::optional<std::string>{});
                    else
                        throw std::runtime_error("invalid command argument(s)");
                }
                else if (command == "m" || command == "metrics")
                    cout << iContext.metrics();
                else if (command == "q" || command == "quit")
                    return false;
                else if (command == "cls" || command == "clear")
                    cout << "\x1B[3J";
                else
                    throw std::runtime_error("unknown command '" + command + "'");
            }
            catch (const neos::language::schema::unresolved_references& e)
            {
                for (auto const& r : e.references)
                    cerr << "Error: " << iContext.schema_source().to_error_text(*r.node, "unresolved schema reference '" + r.symbol + "'") << std::endl;
            }
            catch (const neos::context::warning& e)
            {
                output_compilation_time();
                cerr << "Warning: " << e.what() << std::endl;
            }
            catch (const std::exception& e)
            {
                cerr << "Error: " << e.what() << std::endl;
            }
            catch (...)
            {
                cerr << "Unknown error" << std::endl;
            }
            output().trigger(boost::replace_all_copy(cout.str(), "\n", "\r\n"));
            output().trigger(boost::replace_all_copy(cerr.str(), "\n", "\r\n"));
            cout.str({});
            cerr.str({});
            std::filesystem::current_path(previousCurrentPath);

            return true;
        }
    private:
        std::ostringstream cout;
        std::ostringstream cerr;
        neos::context iContext;
        bool iInteractive;
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
                        aClient = make_ref<neos_session>(neolib::get_service_provider());
                    }
                }
            });
    }

    neos_element_library::~neos_element_library()
    {
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
        static const std::map<std::string, std::function<i_element* (i_project& aProject, const neolib::i_string&)>> sFactoryMethods =
        {
            #define MAKE_ROOT_ELEMENT_FACTORY_FUNCTION(Type) { #Type, [this](i_project& aProject, const neolib::i_string& aElementId) -> i_element* { return new element<Type>{ *this, aProject, #Type, aElementId }; } },
            #define MAKE_NAMED_ROOT_ELEMENT_FACTORY_FUNCTION(Name, Type) { #Name, [this](i_project& aProject, const neolib::i_string& aElementId) -> i_element* { return new element<Type>{ *this, aProject, #Name, aElementId }; } },
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
        static const std::map<std::string, std::function<i_element*(i_element&, const neolib::i_string&, const neolib::i_string&)>> sFactoryMethods =
        {
            #define MAKE_ELEMENT_FACTORY_FUNCTION(Type) { #Type, [this](i_element& aParent, const neolib::i_string& aElementType, const neolib::i_string& aElementId) -> i_element* { return new element<Type>{ *this, aParent, #Type, aElementId, element_group(aElementType) }; } },
            #define MAKE_NAMED_ELEMENT_FACTORY_FUNCTION(Name, Type) { #Name, [this](i_element& aParent, const neolib::i_string& aElementType, const neolib::i_string& aElementId) -> i_element* { return new element<Type>{ *this, aParent, #Name, aElementId, element_group(aElementType) }; } },
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
        static const std::map<std::string, DesignStudio::element_group> sElementGroups =
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
        static std::map<std::string, std::function<void(texture&)>> sIconResources =
        {
        };
        auto existing = sIconResources.find(aElementType.to_std_string());
        if (existing != sIconResources.end())
        {
            if (icons.find(aElementType.to_std_string()) == icons.end())
                existing->second(icons[aElementType.to_std_string()]);
            return icons[aElementType.to_std_string()];
        }
        if (icons.find("default") == icons.end())
            icons["default"] = texture{ size{32.0, 32.0} };
        return icons["default"];
    }
}
