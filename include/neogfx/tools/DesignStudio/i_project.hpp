// i_project.hpp
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

#pragma once

#include <neogfx/core/i_event.hpp>
#include <neogfx/gui/mvc/i_model.hpp>
#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include <neogfx/tools/DesignStudio/i_element.hpp>

namespace neogfx::DesignStudio
{
    class i_project_manager;

    class i_project : public ng::mvc::i_model, public ng::i_reference_counted
    {
    public:
        declare_event(element_added, i_element&);
        declare_event(element_removed, i_element&);
        declare_event(element_moved, i_element&);
    public:
        struct invalid_project_file : std::runtime_error { invalid_project_file(std::string const& aReason) : std::runtime_error{ "neogfx::DesignStudio::i_project::invalid_project_file: " + aReason } {} };
    public:
        typedef i_project abstract_type;
    public:
        virtual i_project_manager& manager() const = 0;
    public:
        virtual void create(const i_string& aName, const i_string& aNamespace) = 0;
        virtual void open(const i_string& aPath) = 0;
        virtual void save(const i_string& aPath) = 0;
        virtual void add_file(const i_string& aPath) = 0; // add an .nrc file to the project (which is then saved as a project file (.dsproj) listing them)
        virtual std::uint32_t file_count() const = 0; // the number of .nrc files in the project
        virtual std::uint32_t element_file(i_element const& aElement) const = 0; // the .nrc file an element is in
    public:
        // translation tables (one per language): "<.nrc file stem>.<language>.xneol" files in the "resources" sub-directory of each of 
        // the project's .nrc files' directories (loaded with them and saved with the project); each translation is of a translatable 
        // string (as in "foo"_t) in a context (optional) for a plurality (optional) (see neogfx::read_translation_file); setting a 
        // translation at translation_count adds one
        virtual std::uint32_t translation_language_count(std::uint32_t aFile) const = 0;
        virtual i_string const& translation_language(std::uint32_t aFile, std::uint32_t aLanguageIndex) const = 0;
        virtual void add_translation_language(std::uint32_t aFile, i_string const& aLanguage) = 0;
        virtual std::uint32_t translation_count(std::uint32_t aFile, i_string const& aLanguage) const = 0;
        virtual void translation(std::uint32_t aFile, i_string const& aLanguage, std::uint32_t aIndex, i_string& aSource, i_string& aContext, i_string& aPlurality, i_string& aTarget) const = 0;
        virtual void set_translation(std::uint32_t aFile, i_string const& aLanguage, std::uint32_t aIndex, i_string const& aSource, i_string const& aContext, i_string const& aPlurality, i_string const& aTarget) = 0;
        // the language the design surface shows the user interface's translatable strings in (empty: untranslated) and a string's 
        // translation into it from the translation tables of the .nrc file an element is in (the string itself if it isn't translated)
        virtual i_string const& design_language() const = 0;
        virtual void set_design_language(i_string const& aLanguage) = 0;
        virtual void design_translation(i_element const& aElement, i_string const& aString, i_string& aResult) const = 0;
    public:
        virtual bool has_path() const = 0;
        virtual const i_string& path() const = 0;
        virtual const i_string& name() const = 0;
        virtual const i_string& namespace_() const = 0;
    public:
        virtual const i_element& root() const = 0;
        virtual i_element& root() = 0;
    public:
        virtual i_element& create_element(i_element& aParent, const i_string& aType, const i_string& aElementId = string{}) = 0;
        virtual void remove_element(i_element& aElement) = 0;
        virtual void move_element(i_element& aElement, i_element& aNewParent, i_element const* aBefore = nullptr) = 0;
    };
}