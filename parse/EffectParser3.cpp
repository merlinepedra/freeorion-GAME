#include "EffectParser3.h"

#include "ValueRefParser.h"
#include "../universe/Condition.h"
#include "../universe/Effects.h"
#include "../universe/Fleet.h"
#include "../universe/ValueRef.h"

#include <boost/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace parse { namespace detail {
    effect_parser_rules_3::effect_parser_rules_3(
        const parse::lexer& tok,
        Labeller& label,
        const condition_parser_grammar& condition_parser,
        const value_ref_grammar<std::string>& string_grammar
    ) :
        effect_parser_rules_3::base_type(start, "effect_parser_rules_3"),
        double_rules(tok, label, condition_parser, string_grammar),
        star_type_rules(tok, label, condition_parser)
    {
        qi::_1_type _1;
        qi::_2_type _2;
        qi::_a_type _a;
        qi::_b_type _b;
        qi::_val_type _val;
        qi::_pass_type _pass;
        qi::omit_type omit_;
        using phoenix::new_;
        using phoenix::construct;
        const boost::phoenix::function<construct_movable> construct_movable_;
        const boost::phoenix::function<deconstruct_movable> deconstruct_movable_;

        move_to
            =    tok.MoveTo_
            >    label(tok.destination_) > condition_parser
            [ _val = construct_movable_(new_<Effect::MoveTo>(deconstruct_movable_(_1, _pass))) ]
            ;

        move_in_orbit
            =    tok.MoveInOrbit_
            >    label(tok.speed_) >  double_rules.expr [ _a = _1 ]
            >   (
                (label(tok.focus_) >  condition_parser [ _val = construct_movable_(new_<Effect::MoveInOrbit>(
                        deconstruct_movable_(_a, _pass),
                        deconstruct_movable_(_1, _pass))) ])
                |
                (
                        label(tok.x_)     >  double_rules.expr [ _b = _1 ]
                    >   label(tok.y_)     >  double_rules.expr [ _val = construct_movable_(new_<Effect::MoveInOrbit>(
                            deconstruct_movable_(_a, _pass),
                            deconstruct_movable_(_b, _pass),
                            deconstruct_movable_(_1, _pass))) ]
                )
            )
            ;

        move_towards
            =    tok.MoveTowards_
            >    label(tok.speed_) > double_rules.expr [ _a = _1 ]
            >    (
                (label(tok.target_) >  condition_parser [ _val = construct_movable_(new_<Effect::MoveTowards>(
                        deconstruct_movable_(_a, _pass),
                        deconstruct_movable_(_1, _pass))) ])
                |
                (
                        label(tok.x_)     > double_rules.expr [ _b = _1 ]
                    >   label(tok.y_)     > double_rules.expr [ _val = construct_movable_(new_<Effect::MoveTowards>(
                            deconstruct_movable_(_a, _pass),
                            deconstruct_movable_(_b, _pass),
                            deconstruct_movable_(_1, _pass))) ]
                )
            )
            ;

        set_destination
            =    tok.SetDestination_
            >    label(tok.destination_) > condition_parser [
                _val = construct_movable_(new_<Effect::SetDestination>(deconstruct_movable_(_1, _pass))) ]
            ;

        set_aggression
            =   tok.SetAggressive_  [ _val = construct_movable_(new_<Effect::SetAggression>(FleetAggression::FLEET_AGGRESSIVE)) ]
            |   tok.SetObstructive_ [ _val = construct_movable_(new_<Effect::SetAggression>(FleetAggression::FLEET_OBSTRUCTIVE)) ]
            |   tok.SetDefensive_   [ _val = construct_movable_(new_<Effect::SetAggression>(FleetAggression::FLEET_DEFENSIVE)) ]
            |   tok.SetPassive_     [ _val = construct_movable_(new_<Effect::SetAggression>(FleetAggression::FLEET_PASSIVE)) ]
            ;

        destroy
            =    tok.Destroy_ [ _val = construct_movable_(new_<Effect::Destroy>()) ]
            ;

        noop
            =    tok.NoOp_ [ _val = construct_movable_(new_<Effect::NoOp>()) ]
            ;

        victory
            =    tok.Victory_
            >    label(tok.reason_) > tok.string [ _val = construct_movable_(new_<Effect::Victory>(_1)) ]
            ;

        add_special_1
            =   tok.AddSpecial_
            >   label(tok.name_) > string_grammar [
                _val = construct_movable_(new_<Effect::AddSpecial>(deconstruct_movable_(_1, _pass))) ]
            ;

        add_special_2
            =  ((omit_[(tok.AddSpecial_ | tok.SetSpecialCapacity_)]
                 >>  label(tok.name_) >> string_grammar
                 >> (label(tok.capacity_) | label(tok.value_))
                )
                >   double_rules.expr
               ) [ _val = construct_movable_(new_<Effect::AddSpecial>(
                   deconstruct_movable_(_1, _pass),
                   deconstruct_movable_(_2, _pass))) ]
            ;

        remove_special
            =   tok.RemoveSpecial_
            >   label(tok.name_) > string_grammar [
                _val = construct_movable_(new_<Effect::RemoveSpecial>(deconstruct_movable_(_1, _pass))) ]
            ;

        add_starlanes
            =   tok.AddStarlanes_
            >   label(tok.endpoint_) > condition_parser [
                _val = construct_movable_(new_<Effect::AddStarlanes>(deconstruct_movable_(_1, _pass))) ]
            ;

        remove_starlanes
            =   tok.RemoveStarlanes_
            >   label(tok.endpoint_) > condition_parser [
                _val = construct_movable_(new_<Effect::RemoveStarlanes>(deconstruct_movable_(_1, _pass))) ]
            ;

        set_star_type
            =   tok.SetStarType_
            >   label(tok.type_) > star_type_rules.expr [
                _val = construct_movable_(new_<Effect::SetStarType>(deconstruct_movable_(_1, _pass))) ]
            ;

        set_texture
            =   tok.SetTexture_
            >   label(tok.name_) > tok.string [ _val = construct_movable_(new_<Effect::SetTexture>(_1)) ]
            ;

        start
            %=  move_to
            |   move_in_orbit
            |   move_towards
            |   set_destination
            |   set_aggression
            |   destroy
            |   noop
            |   victory
            |   add_special_2
            |   add_special_1
            |   remove_special
            |   add_starlanes
            |   remove_starlanes
            |   set_star_type
            |   set_texture
            ;

        move_to.name("MoveTo");
        move_in_orbit.name("MoveInOrbit");
        move_towards.name("MoveTowards");
        set_destination.name("SetDestination");
        set_aggression.name("SetAggression");
        destroy.name("Destroy");
        noop.name("NoOp");
        victory.name("Victory");
        add_special_1.name("AddSpecial");
        add_special_2.name("AddSpecial");
        remove_special.name("RemoveSpecial");
        add_starlanes.name("AddStarlanes");
        remove_starlanes.name("RemoveStarlanes");
        set_star_type.name("SetStarType");
        set_texture.name("SetTexture");

#if DEBUG_EFFECT_PARSERS
        debug(move_to);
        debug(move_in_orbit);
        debug(move_towards)
            debug(set_destination);
        debug(set_aggression);
        debug(destroy);
        debug(noop);
        debug(victory);
        debug(add_special_1);
        debug(add_special_2);
        debug(remove_special);
        debug(add_starlanes);
        debug(remove_starlanes);
        debug(set_star_type);
        debug(set_texture);
#endif
    }
} }
