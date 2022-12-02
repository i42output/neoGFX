// app.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/core/map.hpp>
#include <neogfx/gui/widget/timer.hpp>
#include <neolib/app/application.hpp>
#include <neogfx/core/async_thread.hpp>
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
        struct invalid_options : std::runtime_error { invalid_options(std::string const& aReason) : std::runtime_error("Invalid program options: " + aReason) {} };
    public:
        program_options(int argc, char* argv[]);
    public:
        const boost::program_options::variables_map& options() const final;
        bool console() const final;
        neogfx::renderer renderer() const final;
        std::optional<size_u32> full_screen() const final;
        std::optional<size_u32> dpi_override() const final;
        bool double_buffering() const final;
        bool turbo() const final;
        bool nest() const final;
    private:
        boost::program_options::variables_map iOptions;
    };

    class app : public neolib::application<i_app>, public async_thread, private i_keyboard_handler
    {
        typedef neolib::application<i_app> base_type;
    public:
        define_declared_event(ExecutionStarted, execution_started)
        define_declared_event(NameChanged, name_changed)
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
        typedef neolib::multimap<string, ref_ptr<i_action>> action_list;
        typedef std::vector<i_mnemonic*> mnemonic_list;
    public:
        struct no_instance : std::logic_error { no_instance() : std::logic_error("neogfx::app::no_instance") {} };
        struct action_not_found : std::runtime_error { action_not_found() : std::runtime_error("neogfx::app::action_not_found") {} };
        struct style_not_found : std::runtime_error { style_not_found() : std::runtime_error("neogfx::app::style_not_found") {} };
        struct style_exists : std::runtime_error { style_exists() : std::runtime_error("neogfx::app::style_exists") {} };
    public:
        app(const neolib::i_application_info& aAppInfo);
        template <typename... Args>
        app(Args&&... aArgs) :
            app{ static_cast<const neolib::i_application_info&>(neolib::application_info{ std::forward<Args>(aArgs)... }) } {}
        ~app();
    public:
        static app& instance();
    public:
        const i_program_options& program_options() const noexcept override;
        std::string const& name() const noexcept override;
        void set_name(std::string const& aName) override;
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
        i_style& change_style(std::string const& aStyleName) override;
        i_style& register_style(const i_style& aStyle) override;
    public:
        void clear_translations();
        void load_translations();
        void load_translations(std::filesystem::path const& aTranslationFile);
        i_string const& translate(i_string const& aTranslatableString, i_string const& aContext = string{}, std::int64_t aPlurality = 1) const override;
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
        i_action& add_action(i_ref_ptr<i_action> const& aAction) override;
        i_action& add_action(i_string const& aText) override;
        i_action& add_action(i_string const& aText, i_string const& aImageUri, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::Normal) override;
        i_action& add_action(i_string const& aText, const i_texture& aImage) override;
        i_action& add_action(i_string const& aText, const i_image& aImage) override;
        void remove_action(i_action& aAction) override;
        i_action& find_action(i_string const& aText) override;
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
    protected:
        void idle() override;
    public:
        bool discover(const uuid& aId, void*& aObject) override;
    private:
        bool do_process_events();
    private:
        bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
        bool key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
        bool text_input(i_string const& aText) override;
        bool sys_text_input(i_string const& aText) override;
    private:
        neogfx::program_options iProgramOptions;
        std::unique_ptr<loader> iLoader;
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
        std::map<std::string, std::map<std::string, std::map<std::pair<std::int64_t, std::int64_t>, string>>> iTranslations;
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