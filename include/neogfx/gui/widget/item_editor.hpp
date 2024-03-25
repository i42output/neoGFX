// item_editor.hpp
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

#include "text_edit.hpp"
#include "line_edit.hpp"
#include "spin_box.hpp"

namespace neogfx
{
    class i_item_editor
    {
    public:
        virtual ~i_item_editor() = default;
    public:
        virtual const i_widget& as_widget() const = 0;
        virtual i_widget& as_widget() = 0;
        virtual bool has_text_edit() const = 0;
        virtual text_edit& text_edit() const = 0;
    };

    class item_view;

    template <typename EditorWidget>
    class item_editor : public i_item_editor
    {
    public:
        item_editor(item_view& aParent) :
            iEditorWidget{ std::make_shared<EditorWidget>(aParent) } { init(); }
        item_editor(EditorWidget& aEditorWidget) :
            iEditorWidget{ std::shared_ptr<i_widget>(std::shared_ptr<i_widget>(), &aEditorWidget) } { init(); }
        item_editor(std::shared_ptr<EditorWidget> aEditorWidget) :
            iEditorWidget{ std::shared_ptr<i_widget>(aEditorWidget) } { init(); }
    public:
        const i_widget& as_widget() const override
        {
            return *iEditorWidget;
        }
        i_widget& as_widget() override
        {
            return *iEditorWidget;
        }
        bool has_text_edit() const override;
        neogfx::text_edit& text_edit() const override;
    protected:
        void init()
        {
            static_cast<framed_widget<>&>(as_widget()).set_frame_style(frame_style::SolidFrame);
        }
    private:
        std::shared_ptr<i_widget> iEditorWidget;
    };

    template <>
    inline bool item_editor<text_edit>::has_text_edit() const
    {
        return true;
    }

    template <>
    inline text_edit& item_editor<text_edit>::text_edit() const
    {
        return static_cast<neogfx::text_edit&>(*iEditorWidget);
    }

    template <>
    inline bool item_editor<line_edit>::has_text_edit() const
    {
        return true;
    }

    template <>
    inline text_edit& item_editor<line_edit>::text_edit() const
    {
        return static_cast<neogfx::line_edit&>(*iEditorWidget);
    }

    template <>
    inline bool item_editor<basic_spin_box<int32_t>>::has_text_edit() const
    {
        return true;
    }

    template <>
    inline text_edit& item_editor<basic_spin_box<int32_t>>::text_edit() const
    {
        return static_cast<basic_spin_box<int32_t>&>(*iEditorWidget).text_box();
    }

    template <>
    inline bool item_editor<basic_spin_box<uint32_t>>::has_text_edit() const
    {
        return true;
    }

    template <>
    inline text_edit& item_editor<basic_spin_box<uint32_t>>::text_edit() const
    {
        return static_cast<basic_spin_box<uint32_t>&>(*iEditorWidget).text_box();
    }

    template <>
    inline bool item_editor<basic_spin_box<int64_t>>::has_text_edit() const
    {
        return true;
    }

    template <>
    inline text_edit& item_editor<basic_spin_box<int64_t>>::text_edit() const
    {
        return static_cast<basic_spin_box<int64_t>&>(*iEditorWidget).text_box();
    }

    template <>
    inline bool item_editor<basic_spin_box<uint64_t>>::has_text_edit() const
    {
        return true;
    }

    template <>
    inline text_edit& item_editor<basic_spin_box<uint64_t>>::text_edit() const
    {
        return static_cast<basic_spin_box<uint64_t>&>(*iEditorWidget).text_box();
    }

    template <>
    inline bool item_editor<basic_spin_box<float>>::has_text_edit() const
    {
        return true;
    }

    template <>
    inline text_edit& item_editor<basic_spin_box<float>>::text_edit() const
    {
        return static_cast<basic_spin_box<float>&>(*iEditorWidget).text_box();
    }

    template <>
    inline bool item_editor<basic_spin_box<double>>::has_text_edit() const
    {
        return true;
    }

    template <>
    inline text_edit& item_editor<basic_spin_box<double>>::text_edit() const
    {
        return static_cast<basic_spin_box<double>&>(*iEditorWidget).text_box();
    }
}