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

#include <neolib/core/vector.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include <neogfx/tools/DesignStudio/i_node.hpp>

namespace neogfx::DesignStudio
{
    template <typename Base, pin_type Type>
    class node_pin : public reference_counted<Base>
    {
        typedef Base base_type;
    public:
        node_pin(i_node& aNode) :
            iNode{ aNode }
        {
        }
    public:
        pin_type type() const override
        {
            return Type;
        }
        i_node& get() const override
        {
            return iNode;
        }
        bool connected() const override
        {
            for (auto const& c : get().connections())
                if (&c->source() == this || &c->destination() == this)
                    return true;
            return false;
        }
    public:
        neogfx::color color() const override
        {
            return as_widget().color();
        }
        i_pin_widget const& as_widget() const override
        {
            return *iWidget;
        }
        i_pin_widget& as_widget() override
        {
            return *iWidget;
        } 
        void set_widget(i_pin_widget& aWidget) override
        {
            iWidget = &aWidget;
        }
    private:
        i_node& iNode;
        i_pin_widget* iWidget = nullptr;
    };

    class node_input_pin : public node_pin<i_node_input_pin, pin_type::Input>
    {
        typedef node_pin<i_node_input_pin, pin_type::Input> base_type;
    public:
        using base_type::base_type;
    };

    class node_output_pin : public node_pin<i_node_output_pin, pin_type::Output>
    {
        typedef node_pin<i_node_output_pin, pin_type::Output> base_type;
    public:
        using base_type::base_type;
    };

    class node_connection : public reference_counted<i_node_connection>
    {
    public:
        node_connection(i_node_pin& aSource, i_node_pin& aDestination) : 
            iSource{ aSource },
            iDestination{ aDestination }
        {
        }
    public:
        i_node_pin const& source() const override
        {
            return *iSource;
        }
        i_node_pin& source() override
        {
            return *iSource;
        }
        i_node_pin const& destination() const override
        {
            return *iDestination;
        }
        i_node_pin& destination() override
        {
            return *iDestination;
        }
    private:
        ref_ptr<i_node_pin> iSource;
        ref_ptr<i_node_pin> iDestination;
    };

    class node : public i_node
    {
        typedef node self_type;
        typedef i_node base_type;
    public:
        define_declared_event(InputAdded, input_added, i_node_input_pin&)
        define_declared_event(InputRemoved, input_removed, i_node_input_pin&)
        define_declared_event(OutputAdded, output_added, i_node_output_pin&)
        define_declared_event(OutputRemoved, output_removed, i_node_output_pin&)
        define_declared_event(ConnectionAdded, connection_added, i_node_connection&)
        define_declared_event(ConnectionRemoved, connection_removed, i_node_connection&)
    public:
        typedef base_type abstract_type;
        typedef neolib::vector<ref_ptr<i_node_input_pin>> node_inputs_t;
        typedef neolib::vector<ref_ptr<i_node_output_pin>> node_outputs_t;
        typedef neolib::vector<ref_ptr<i_node_connection>> node_connections_t;
    public:
        node_inputs_t const& inputs() const override
        {
            return iInputs;
        }
        node_outputs_t const& outputs() const override
        {
            return iOutputs;
        }
        node_connections_t const& connections() const override
        {
            return iConnections;
        }
        std::size_t connected_inputs() const override
        {
            std::size_t result = 0;
            for (auto& c : connections())
                if (&c->destination().get() == this && c->destination().type() == pin_type::Input)
                    ++result;
            return result;
        }
        std::size_t connected_outputs() const override
        {
            std::size_t result = 0;
            for (auto& c : connections())
                if (&c->source().get() == this && c->source().type() == pin_type::Output)
                    ++result;
            return result;
        }
    public:
        void add_input(i_node_input_pin& aPin) override
        {
            iInputs.emplace_back(aPin);
            InputAdded.trigger(*iInputs.back());
        }
        void remove_input(i_node_input_pin& aPin) override
        {
            auto existing = std::find_if(iInputs.begin(), iInputs.end(), [&](auto&& e) { return &*e == &aPin; });
            if (existing != iInputs.end())
            {
                ref_ptr<i_node_input_pin> temp = *existing;
                iInputs.erase(existing);
                InputRemoved.trigger(*temp);
            }
        }
        void add_output(i_node_output_pin& aPin) override
        {
            iOutputs.emplace_back(aPin);
            OutputAdded.trigger(*iOutputs.back());
        }
        void remove_output(i_node_output_pin& aPin) override
        {
            auto existing = std::find_if(iOutputs.begin(), iOutputs.end(), [&](auto&& e) { return &*e == &aPin; });
            if (existing != iOutputs.end())
            {
                ref_ptr<i_node_output_pin> temp = *existing;
                iOutputs.erase(existing);
                OutputRemoved.trigger(*temp);
            }
        }
        void add_connection(i_node_pin& aSource, i_node_pin& aDestination) override
        {
            iConnections.push_back(make_ref<node_connection>(aSource, aDestination));
            ConnectionAdded.trigger(*iConnections.back());
        }
        void remove_connection(i_node_pin& aSource, i_node_pin& aDestination) override
        {
            auto existing = std::find_if(iConnections.begin(), iConnections.end(), [&](auto&& e) { return &e->source() == &aSource && &e->destination() == &aDestination; });
            if (existing != iConnections.end())
            {
                ref_ptr<node_connection> temp = *existing;
                iConnections.erase(existing);
                ConnectionRemoved.trigger(*temp);
            }
        }
    private:
        node_inputs_t iInputs;
        node_outputs_t iOutputs;
        node_connections_t iConnections;
    };
}