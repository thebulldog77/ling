/**
 * @file    parser.cpp
 * @author  Jacky Alcine <jackyalcine@gmail.com>
 * @date    June 14, 2011 11:34 PM
 * @license GPL3
 *
 * @legalese
 * Copyright (c) SII 2010 - 2011
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * @endlegalese
 */

#ifndef __PARSER_HPP__
#define __PARSER_HPP__
#define SKIP_AUTOMOC true

#include <string>
#include <vector>
#include <map>
#include <QString>
#include <QtXml/QDomDocument>
#include <wntrdata.hpp>
#include <syntax.hpp>

using namespace std;

using std::string;
using std::vector;
using std::map;

namespace Wintermute {
    namespace Linguistics {
        struct Parser;
        struct Meaning;
        struct RuleSet;
        struct Rule;
        struct Binding;

        /**
         * @brief Represents a vector of vector of nodes.
         * @typedef NodeTree
         */
        typedef vector<NodeVector> NodeTree;

        /**
         * @brief Represents a collection of meanings.
         * @typedef MeaningVector
         */
        typedef vector<const Meaning*> MeaningVector;

        /**
         * @brief
         * @typedef RuleList
         */
        typedef vector<Rule*> RuleVector;

        /**
         * @brief
         *
         * @typedef BindingVector
         */
        typedef vector<Binding*> BindingVector;

        /**
         * @brief
         * @typedef RuleSetMap
         */
        typedef map<const string, RuleSet*> RuleSetMap;

        /**
         * @brief Represents the potential connection of words by a specified rule as defined by its parent rule.
         * @class Binding parser.hpp "include/wntr/ling/parser.hpp"
         */
        class Binding : public QObject {
            Q_OBJECT
            friend class Rule;
            public:
                ~Binding();
                static const Binding* obtain ( const Node&, const Node& );
                const Rule* parentRule() const;
                const bool canBind ( const Node&, const Node& ) const;
                const QString getProperty ( const QString& ) const;
                const Link* bind ( const Node&, const Node& ) const;

            protected:
                Binding ( QDomElement , const Rule* );
                Binding();

            private:
                QDomElement m_ele;
                const Rule* m_rl;
        };

        /**
         * @brief Represents a set of bindings that permit linguistics links to be converted into ontological links.
         * @class Rule parser.hpp "include/wntr/ling/parser.hpp"
         */
        class Rule : public QObject {
            Q_OBJECT
            Q_PROPERTY(string type READ type)

            friend class RuleSet;
            public:
                ~Rule();
                /**
                 * @brief Copy constructor.
                 * @fn Rule
                 * @param Rule The original Node.
                 */
                Rule ( const Rule& );
                /**
                 * @brief Returns a Rule that's satisified by this Node.
                 * @fn obtain
                 * @param Node A qualifying Node.
                 */
                static const Rule* obtain ( const Node & );
                /**
                 * @brief Determines if this Node can be binded with a Node that falls under this rule.
                 * @fn canBind
                 * @param Node The source Node in question.
                 * @param Node The destination Node in question.
                 */
                const bool canBind ( const Node & , const Node & ) const;
                /**
                 * @brief Creates a Link between a qualifying source Node and a destination Node.
                 * @fn bind
                 * @param Node The source Node in question.
                 * @param Node The destination Node in question.
                 */
                const Link* bind ( const Node &, const Node & ) const;
                /**
                 * @brief Returns a string representing the type of nodes that this Rule looks for.
                 * @fn type
                 */
                const string type() const;
                /**
                 * @brief Determines if a Node is qualified to use this Rule.
                 * @fn appliesFor
                 * @param Node The source node in question.
                 */
                const bool appliesFor ( const Node& ) const;
                /**
                 * @brief Returns a pointer to the Binding that works for the two specified Nodes.
                 * @fn getBindingFor
                 * @param Node The source Node in question.
                 * @param Node The destination Node in question.
                 */
                const Binding* getBindingFor ( const Node&, const Node& ) const;
                /**
                 * @brief Returns a pointer to the RuleSet this originated from.
                 * @fn parentRuleSet
                 */
                const RuleSet* parentRuleSet() const;

            protected:
                /**
                 * @brief Constructor.
                 * @fn Rule
                 * @param QDomElement The element that contains the binding data.
                 * @param RuleSet The parent RuleSet.
                 */
                Rule ( QDomElement , const RuleSet* );
                /**
                 * @brief Empty constructor.
                 * @fn Rule
                 */
                Rule();

            private:
                QDomElement m_ele;
                const RuleSet* m_rlst;
                BindingVector m_bndVtr;
        };

        /**
         * @brief Represents a source of obtaining rules and directly forming bindings with linguistics information.
         * This class manages the obtaining of rules from a certain locale.
         * @class RuleSet parser.hpp "include/wntr/ling/parser.hpp"
         * @todo Define methods for creating new rules and bindings.
         */
        class RuleSet : public QObject {
            Q_OBJECT

            Q_PROPERTY(string locale READ locale)

            public:
                ~RuleSet();
                static RuleSet* obtain ( const Node& );
                /**
                 * @brief Returns the Rule that can be used to link with another node.
                 * @fn getRuleFor
                 * @param Node The source node to search for a Rule with.
                 */
                const Rule* getRuleFor ( const Node& ) const;
                /**
                 * @brief Returns the Binding that can link the two specified nodes.
                 * @fn getBindingFor
                 * @param Node The source node to search for a Rule with.
                 * @param Node The destination node to search against the possible Binding with.
                 */
                const Binding* getBindingFor ( const Node& , const Node& ) const;
                /**
                 * @brief Determines if a Link can be created between the supplied nodes.
                 * @fn canBind
                 * @param Node The source node for linking in question.
                 * @param Node The destination node for linking in question.
                 */
                const bool canBind ( const Node& , const Node & ) const;
                /**
                 * @brief Creates a Link between two nodes based on the rules in this RuleSet.
                 * @fn bind
                 * @param Node The source node for linking.
                 * @param Node The destination node for linking.
                 */
                const Link* bind ( const Node&, const Node & ) const;
                /**
                 * @brief The locale in use.
                 * Returns the value of the locale containing the rules for this RuleSet.
                 * @fn locale
                 */
                const string locale() const;

            protected:
                /**
                 * @brief
                 * @fn RuleSet
                 */
                RuleSet();

                /**
                 * @brief
                 * @fn RuleSet
                 * @param
                 */
                RuleSet ( const string& );

            private:
                /**
                 * @brief
                 *
                 * @fn __init
                 */
                void __init ( const string& = Wintermute::Data::Linguistics::Configuration::locale () );

                QDomDocument* m_dom;
                RuleVector* m_rules;
                static RuleSetMap s_rsm;
        };

        /**
         * @brief Encapsulates the primary object used to cast a simple string representing a bit of language into machine-interpretable ontological information.
         * @class Parser parser.hpp "include/wntr/ling/parser.hpp"
         */
        class Parser : public QObject {
            Q_OBJECT

            Q_PROPERTY(string locale READ locale WRITE setLocale)

            public:
                Parser ( const string& );
                const string locale() const;
                void setLocale ( const string& = Wintermute::Data::Linguistics::Configuration::locale ());
                void parse ( const string& );
                void process ( const string& );
            protected:
                Parser();
                mutable string m_lcl;
            private:
                StringVector getTokens ( const string& );
                NodeVector formNodes ( StringVector& );
                NodeTree expandNodes ( NodeVector& );
                NodeTree expandNodes ( NodeTree& , const int& = 0, const int& = 0 );
                const Meaning formMeaning ( const NodeVector& );
                static const string formShorthand ( const NodeVector& , const Node::FormatDensity& = Node::FULL );
        };

        /**
         * @brief Represents the end-result of linguistics linking and provides an easier means of glancing into the linking process.
         * @class Meaning parser.hpp "include/wntr/ling/parser.hpp"
         */
        class Meaning : public QObject {
            Q_OBJECT

            Q_PROPERTY(const Link* base READ base)
            Q_PROPERTY(const LinkVector* siblings READ siblings)
            public:
                ~Meaning();
                /**
                 * @brief The root link that can be determined as the base, underlying Meaning of the parsed text.
                 * @fn base
                 */
                const Link* base() const;
                const LinkVector* siblings() const;
                const string toText() const;
                const LinkVector* isLinkedTo(const Node& ) const;
                const LinkVector* isLinkedBy(const Node& ) const;
                static const Meaning* form ( const NodeVector& , LinkVector* = new LinkVector );
                static const Meaning* form ( const Link* = NULL, const LinkVector* = NULL );

            protected:
                Meaning();
                Meaning ( const Link* = NULL, const LinkVector* = NULL );

            private:
                const Link* m_lnk;
                const LinkVector* m_lnkVtr;
        };
    }
}

#endif /* __PARSER_HPP__ */
// kate: indent-mode cstyle; space-indent on; indent-width 4;
