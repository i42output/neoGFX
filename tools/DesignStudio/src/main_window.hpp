// main_window.hpp
/*
neoGFX Design Studio
Copyright(C) 2020 Leigh Johnston

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

#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/view/view_container.hpp>
#include <neogfx/gui/widget/dock.hpp>
#include <neogfx/gui/widget/dockable.hpp>
#include <neogfx/gui/widget/table_view.hpp>
#include <neogfx/core/css.hpp>
#include <neogfx/gui/dialog/settings_dialog.hpp>
#include <neogfx/app/file_dialog.hpp>
#include <neogfx/app/i_clipboard.hpp>
#include <neogfx/tools/DesignStudio/project_manager.hpp>
#include <neogfx/tools/DesignStudio/project.hpp>
#include <neogfx/tools/DesignStudio/settings.hpp>
#include <neogfx/tools/DesignStudio/i_ide.hpp>
#include "new_project_dialog.hpp"
#include "toolbox_model.hpp"
#include "workflow_model.hpp"
#include "object_model.hpp"
#include "DesignStudio.ui.hpp"

namespace neogfx::DesignStudio
{
    class main_window_ex : public main_window, public reference_counted<i_ide>, private i_clipboard_sink
    {
    public:
        neolib::i_setting& autoscaleDocks;
        neolib::i_setting& workspaceSize;
        neolib::i_setting& workspacePosition;
        neolib::i_setting& leftDockWidth;
        neolib::i_setting& rightDockWidth;
        neolib::i_setting& leftDockWeight;
        neolib::i_setting& rightDockWeight;
        neolib::i_setting& workspaceFont;
        neolib::i_setting& subpixelRendering;
        neolib::i_setting& toolbarIconSize;
        neolib::i_setting& themeColor;
        neolib::i_setting& workspaceGridType;
        neolib::i_setting& workspaceGridSize;
        neolib::i_setting& workspaceGridSubdivisions;
        neolib::i_setting& workspaceGridColor;
    public:
        main_window_ex(main_app& aApp, settings& aSettings, project_manager& aProjectManager);
    protected:
        void close() override;
    protected:
        void add_action(uuid const& aMenuId, i_action& aAction) override;
        void add_sub_menu(uuid const& aMenuId, i_menu& aSubMenu) override;
    protected:
        bool can_undo() const override;
        bool can_redo() const override;
        bool can_cut() const override;
        bool can_copy() const override;
        bool can_paste() const override;
        bool can_delete_selected() const override;
        bool can_select_all() const override;
        void undo(i_clipboard& aClipboard) override;
        void redo(i_clipboard& aClipboard) override;
        void cut(i_clipboard& aClipboard) override;
        void copy(i_clipboard& aClipboard) override;
        void paste(i_clipboard& aClipboard) override;
        void delete_selected() override;
        void select_all() override;
    private:
        void paint_workspace(ng::i_graphics_context& aGc);
    private:
        project_manager& iProjectManager;
        ng::dock iLeftDock;
        ng::dock iRightDock;
        ng::dockable iToolbox;
        ng::dockable iWorkflow;
        ng::dockable iObjects;
        ng::dockable iProperties;
        ng::horizontal_layout iWorkspaceLayout;
        ng::scrollable_view_container iWorkspace;
        ng::texture iBackgroundTexture1;
        ng::texture iBackgroundTexture2;
        toolbox_model iToolboxModel;
        toolbox_presentation_model iToolboxPresentationModel;
        workflow_model iWorkflowModel;
        workflow_presentation_model iWorkflowPresentationModel;
        object_model iObjectModel;
        ng::item_selection_model iObjectSelectionModel;
        object_presentation_model iObjectPresentationModel;
        ng::sink iSink;
    };
}

