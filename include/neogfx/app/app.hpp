// app.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <map>
#include <optional>
#include <boost/pool/pool_alloc.hpp>
#include <neolib/async_thread.hpp>

#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/app/event_processing_context.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/app/style.hpp>
#include <neogfx/app/action.hpp>
#include <neogfx/app/i_mnemonic.hpp>
#include <neogfx/app/i_help.hpp>

#ifdef _WIN32
#pragma comment(linker, "/include:nrc_neogfx_icons")
#pragma comment(linker, "/include:nrc_neogfx_resources")
#endif

namespace neogfx
{
    class program_options : public i_program_options
    {
        struct invalid_options : std::runtime_error { invalid_options(const std::string& aReason) : std::runtime_error("Invalid program options: " + aReason) {} };
    public:
        program_options(int argc, char* argv[]);
    public:
        const boost::program_options::variables_map& options() const override;
        bool debug() const override;
        neogfx::renderer renderer() const override;
        std::optional<std::pair<uint32_t, uint32_t>> full_screen() const override;
        bool double_buffering() const override;
        bool nest() const override;
    private:
        boost::program_options::variables_map iOptions;
    };

    class app : public neolib::async_thread, public i_app, private i_keyboard_handler
    {
    public:
        define_declared_event(ExecutionStarted, execution_started)
        define_declared_event(CurrentStyleChanged, current_style_changed, style_aspect)
    private:
        class loader
        {
        public:
            loader(const neogfx::program_options& aProgramOptions, app& aApp);
            ~loader();
        private:
            app& iApp;
        };
    private:
        typedef std::map<std::string, style> style_list;
        typedef std::multimap<std::string, std::shared_ptr<i_action>> action_list;
        typedef std::vector<i_mnemonic*> mnemonic_list;
    public:
        struct no_instance : std::logic_error { no_instance() : std::logic_error("neogfx::app::no_instance") {} };
        struct action_not_found : std::runtime_error { action_not_found() : std::runtime_error("neogfx::app::action_not_found") {} };
        struct style_not_found : std::runtime_error { style_not_found() : std::runtime_error("neogfx::app::style_not_found") {} };
        struct style_exists : std::runtime_error { style_exists() : std::runtime_error("neogfx::app::style_exists") {} };
    public:
        app(const std::string& aName = std::string());
        app(int argc, char* argv[], const std::string& aName = std::string());
        ~app();
    public:
        static app& instance();
        const i_program_options& program_options() const override;
        const std::string& name() const override;
        int exec(bool aQuitWhenLastWindowClosed = true) override;
        bool in_exec() const override;
        void quit(int aResultCode = 0) override;
    public:
        dimension default_dpi_scale_factor() const override;
    public:
        const i_texture& default_window_icon() const override;
        void set_default_window_icon(const i_texture& aIcon) override;
        void set_default_window_icon(const i_image& aIcon) override;
        const i_style& current_style() const override;
        i_style& current_style() override;
        i_style& change_style(const std::string& aStyleName) override;
        i_style& register_style(const i_style& aStyle) override;
    public:
        const std::string& translate(const std::string& aTranslatableString, const std::string& aContext = std::string{}) const override;
    public:
        i_action& action_file_new() override;
        i_action& action_file_open() override;
        i_action& action_file_close() override;
        i_action& action_file_close_all() override;
        i_action& action_file_save() override;
        i_action& action_file_save_all() override;
        i_action& action_file_exit() override;
        i_action& action_undo() override;
        i_action& action_redo() override;
        i_action& action_cut() override;
        i_action& action_copy() override;
        i_action& action_paste() override;
        i_action& action_delete() override;
        i_action& action_select_all() override;
        i_action& add_action(i_action& aAction) override;
        i_action& add_action(const std::string& aText) override;
        i_action& add_action(const std::string& aText, const std::string& aImageUri, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::Normal) override;
        i_action& add_action(const std::string& aText, const i_texture& aImage) override;
        i_action& add_action(const std::string& aText, const i_image& aImage) override;
        void remove_action(i_action& aAction) override;
        i_action& find_action(const std::string& aText) override;
        void add_mnemonic(i_mnemonic& aMnemonic) override;
        void remove_mnemonic(i_mnemonic& aMnemonic) override;
    public:
        i_menu& add_standard_menu(i_menu& aParentMenu, standard_menu aStandardMenu) override;
    public:
        i_help& help() const override;
    public:
        bool process_events() override;
        bool process_events(i_event_processing_context& aContext) override;
        i_event_processing_context& event_processing_context() override;
    private:
        bool do_process_events();
    private:
        bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
        bool key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
        bool text_input(const std::string& aText) override;
        bool sys_text_input(const std::string& aText) override;
    private:
        neolib::async_event_queue iAsyncEventQueue;
        neogfx::program_options iProgramOptions;
        loader iLoader;
        std::string iName;
        bool iQuitWhenLastWindowClosed;
        bool iInExec;
        std::optional<int> iQuitResultCode;
        texture iDefaultWindowIcon;
        style_list iStyles;
        style_list::iterator iCurrentStyle;
        action_list iActions;
        neolib::callback_timer iStandardActionManager;
        mnemonic_list iMnemonics;
        neogfx::event_processing_context iAppContext;
        std::vector<std::pair<key_code_e, key_modifiers_e>> iKeySequence;
        mutable std::unique_ptr<i_help> iHelp;
        // standard actions
    public:
        action actionFileNew;
        action actionFileOpen;
        action actionFileClose;
        action actionFileCloseAll;
        action actionFileSave;
        action actionFileSaveAll;
        action actionFileExit;
        action actionUndo;
        action actionRedo;
        action actionCut;
        action actionCopy;
        action actionPaste;
        action actionDelete;
        action actionSelectAll;
    };
}