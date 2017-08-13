// item_editor.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2017 Leigh Johnston
  
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
		virtual ~i_item_editor() {}
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
			iEditorWidget{ std::make_shared<EditorWidget>(aParent) } {}
		item_editor(EditorWidget& aEditorWidget) :
			iEditorWidget{ std::shared_ptr<i_widget>(std::shared_ptr<i_widget>(), &aEditorWidget) } {}
		item_editor(std::shared_ptr<EditorWidget> aEditorWidget) :
			iEditorWidget{ std::shared_ptr<i_widget>(aEditorWidget) } {}
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
	inline bool item_editor<spin_box>::has_text_edit() const
	{
		return true;
	}

	template <>
	inline text_edit& item_editor<spin_box>::text_edit() const
	{
		return static_cast<spin_box&>(*iEditorWidget).text_box();
	}
}