// i_node.hpp
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

#include <neolib/core/i_vector.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include <neogfx/tools/DesignStudio/i_element.hpp>

namespace neogfx::DesignStudio
{
    class i_pin_widget
    {
    public:
        virtual i_widget const& icon() const = 0;
        virtual i_widget& icon() = 0;
    };

    enum class pin_type
    {
        Execution = 0x01,
        Data = 0x02,
        Input = 0x10,
        Output = 0x20
    };

    class i_node;

    class i_node_pin : public ng::i_reference_counted
    {
        typedef i_node_pin self_type;
    public:
        typedef self_type abstract_type;
    public:
        virtual pin_type type() const = 0;
        virtual i_node& get() const = 0;
        virtual i_pin_widget const& as_widget() const = 0;
        virtual i_pin_widget& as_widget() = 0;
        virtual void set_widget(i_pin_widget& aWidget) = 0;
    };

    class i_node_input_pin : public i_node_pin
    {
        typedef i_node_input_pin self_type;
    public:
        typedef self_type abstract_type;
    };

    class i_node_output_pin : public i_node_pin
    {
        typedef i_node_output_pin self_type;
    public:
        typedef self_type abstract_type;
    };

    class i_node_connection : public ng::i_reference_counted
    {
        typedef i_node_connection self_type;
    public:
        typedef self_type abstract_type;
    public:
        virtual i_node_pin const& source() const = 0;
        virtual i_node_pin& source() = 0;
        virtual i_node_pin const& destination() const = 0;
        virtual i_node_pin& destination() = 0;
    };

    class i_node : public i_element
    {
        typedef i_node self_type;
    public:
        declare_event(input_added, i_node_input_pin&)
        declare_event(input_removed, i_node_input_pin&)
        declare_event(output_added, i_node_output_pin&)
        declare_event(output_removed, i_node_output_pin&)
        declare_event(connection_added, i_node_connection&)
        declare_event(connection_removed, i_node_connection&)
    public:
        typedef i_element abstract_type;
        typedef neolib::i_vector<i_ref_ptr<i_node_input_pin>> node_inputs_t;
        typedef neolib::i_vector<i_ref_ptr<i_node_output_pin>> node_outputs_t;
        typedef neolib::i_vector<i_ref_ptr<i_node_connection>> node_connections_t;
    public:
        virtual node_inputs_t const& inputs() const = 0;
        virtual node_outputs_t const& outputs() const = 0;
        virtual node_connections_t const& connections() const = 0;
        virtual std::size_t connected_inputs() const = 0;
        virtual std::size_t connected_outputs() const = 0;
    public:
        virtual void add_input(i_node_input_pin& aPin) = 0;
        virtual void remove_input(i_node_input_pin& aPin) = 0;
        virtual void add_output(i_node_output_pin& aPin) = 0;
        virtual void remove_output(i_node_output_pin& aPin) = 0;
        virtual void add_connection(i_node_pin& aSource, i_node_pin& aDestination) = 0;
        virtual void remove_connection(i_node_pin& aSource, i_node_pin& aDestination) = 0;
    };

    inline void connect_nodes(i_node_pin& aSource, i_node_pin& aDestination)
    {
        aSource.get().add_connection(aSource, aDestination);
        aDestination.get().add_connection(aSource, aDestination);
    }
}