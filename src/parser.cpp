/**
 * @file    parser.cpp
 * @author  Jacky Alcine <jackyalcine@gmail.com>
 * @date    June 14, 2011, 11:34 PM
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
 * along with this program.  If not, see <http://www.gnu.orgccc/licenses/>.
 * @endlegalese
 *
 * @todo Remove the connection to QtXml in this code and implement the read/write capabilities of RuleSets in WntrData::Linguistics.
 */

#include "syntax.hpp"
#include "parser.hpp"
#include <wntrdata.hpp>
#include <iostream>
#include <iomanip>
#include <boost/tokenizer.hpp>
#include <QString>
#include <QVector>
#include <QTextStream>
#include <QFile>

using namespace boost;
using namespace std;

using std::cout;
using std::endl;

namespace Wintermute {
    namespace Linguistics {
        RuleSetMap RuleSet::s_rsm;

        Binding::Binding() { }

        Binding::Binding(QDomElement p_ele, const Rule* p_rl) : m_rl(p_rl), m_ele(p_ele) { }

        /// @todo Should it check if its parent Rule accepts it? (Idts.)
        /// @todo Check against the 'with' attribute and determine if it exists in there.
        const bool Binding::canBind (const Node &p_nd, const Node& p_nd2) const {
            if (!this->parentRule ()->appliesFor (p_nd))
                return false;

            const QString l_wh = this->m_ele.attribute ("with");
            const QString l_ndStr(p_nd2.toString (Node::EXTRA).c_str ());
            QStringList l_options;

            if (l_wh.contains (","))
                l_options = l_wh.split (",");
            else
                l_options += l_wh;

            for (QStringList::ConstIterator l_itr = l_options.begin (); l_itr != l_options.end (); l_itr++){
                const QString l_s = *l_itr;
                if (l_ndStr.contains (l_s) || l_ndStr.indexOf (l_s) != -1) return true;
            }

            cout << p_nd.toString (Node::EXTRA) << " " << qPrintable(l_ndStr) << endl;

            return false;
        }

        /// @todo Form a link with the type being the Rule's type, locale being the rule's locale, and p_nd1 being the source node.
        const Link* Binding::bind (const Node& p_nd1, const Node& p_nd2) const {
            string l_type = this->parentRule ()->type ();
            string l_lcl = this->parentRule ()->parentRuleSet ()->locale ();
            Node *l_nd = const_cast<Node*>(&p_nd1), *l_nd2 = const_cast<Node*>(&p_nd2);

            if (m_ele.attribute ("postAction").contains ("reverse")){
                l_type = p_nd2.toString (Node::MINIMAL);
                l_lcl = p_nd2.locale ();
                Node *l_tmp = l_nd;
                l_nd = l_nd2;
                l_nd2 = l_tmp;
                cout << "(ling) [Binding] Reversed the type and locale of the link." << endl;
            }
            return Link::form(static_cast<const FlatNode*>(l_nd), static_cast<const FlatNode*>(l_nd2),
                            l_type, l_lcl);
        }

        const Rule* Binding::parentRule () const { return m_rl; }

        Rule::Rule() { }

        Rule::Rule(QDomElement p_ele, const RuleSet* p_rlst) : m_ele(p_ele), m_rlst(p_rlst) {
            QDomNodeList l_ndlst = m_ele.elementsByTagName ("Bind");

            for (int i = 0; i < l_ndlst.count (); i++){
                QDomElement l_e = l_ndlst.at (i).toElement ();
                m_bndVtr.push_back ((new Binding(l_e,this)));
            }
        }

        const Rule* Rule::obtain(const Node& p_nd) {
            const RuleSet* l_rlst = RuleSet::obtain (p_nd);
            return l_rlst->getRuleFor(p_nd);
        }

        /// @todo Iterate across its BindingVector and find a good one.
        const Link* Rule::bind(const Node& p_curNode, const Node& p_nextNode) const {
            for (BindingVector::const_iterator i = m_bndVtr.begin (); i != m_bndVtr.end (); i++){
                const Binding* l_bnd = *i;
                if (l_bnd->canBind(p_curNode,p_nextNode))
                    return l_bnd->bind(p_curNode,p_nextNode);
            }

            return NULL;
        }

        /// @todo Question: Should the last word have a chance at linking?
        const bool Rule::canBind(const Node& p_nd, const Node &p_dstNd) const {
            for (BindingVector::const_iterator i = m_bndVtr.begin (); i != m_bndVtr.end (); i++){
                const Binding* l_bnd = *i;
                if (l_bnd->canBind(p_nd,p_dstNd))
                    return true;
            }

            return false;
        }

        const Binding* Rule::getBindingFor(const Node& p_nd, const Node& p_nd2) const {
            for (BindingVector::const_iterator i = m_bndVtr.begin (); i != m_bndVtr.end (); i++){
                const Binding* l_bnd = *i;
                if (l_bnd->canBind(p_nd,p_nd2))
                    return l_bnd;
            }

            return NULL;
        }

        const bool Rule::appliesFor(const Node& p_nd) const {
            const QString l_ndStr(p_nd.toString (Node::EXTRA).c_str ());
            const QString l_rlStr(type().c_str ());

            return (l_ndStr.contains (l_rlStr) || l_ndStr.indexOf (l_rlStr) != -1);
        }

        const string Rule::type() const { return m_ele.attribute("type").toStdString (); }

        const RuleSet* Rule::parentRuleSet () const { return m_rlst; }


        RuleSet::RuleSet() : m_dom((new QDomDocument)), m_rules((new RuleVector)) { __init (); }

        RuleSet::RuleSet(const string &p_lcl) : m_dom((new QDomDocument)), m_rules((new RuleVector)) { __init(p_lcl); }

        void RuleSet::__init(const string& p_lcl) {
            const string l_dir = Data::Linguistics::Configuration::directory () + string("/locale/") + p_lcl + string("/rules.dat");
            QFile *l_rlstDoc = new QFile(QString(l_dir.c_str ()));

            l_rlstDoc->open(QIODevice::ReadOnly);

            m_dom->setContent(l_rlstDoc);
            QDomElement l_ele = m_dom->documentElement ();
            QDomNode l_domNd = l_ele.firstChild ();

            if (!m_rules->empty()) m_rules->clear();

            while (!l_domNd.isNull ()) {
                if (l_domNd.isElement ()){
                    QDomElement l_curEle = l_domNd.toElement ();
                    if (l_curEle.tagName () == "Rule")
                        m_rules->push_back(new Rule(l_curEle,this));
                }

                l_domNd = l_domNd.nextSibling ();
            }

            //cout << "(ling) [RuleSet] Loaded " << m_rules->size () << " rules." << endl;
        }

        RuleSet* RuleSet::obtain(const Node& p_ndRef) {
            RuleSet* l_rlst = NULL;

            if (s_rsm.find(p_ndRef.locale ()) == s_rsm.end()) {
                l_rlst = new RuleSet(p_ndRef.locale ());
                s_rsm.insert(RuleSetMap::value_type(p_ndRef.locale (),l_rlst));
            }
            else
                l_rlst = s_rsm.find(p_ndRef.locale ())->second;

            return l_rlst;
        }

        const Rule* RuleSet::getRuleFor(const Node& p_nd) const {
            for (RuleVector::const_iterator i = m_rules->begin (); i != m_rules->end (); i++){
                const Rule* l_rl = *i;
                if (l_rl->appliesFor(p_nd))
                    return l_rl;
            }

            return NULL;
        }

        const Binding* RuleSet::getBindingFor(const Node& p_nd, const Node& p_nd2) const {
            const Rule* l_rl = getRuleFor(p_nd);
            if (l_rl)
                return l_rl->getBindingFor(p_nd,p_nd2);
            else
                return NULL;
        }

        const bool RuleSet::canBind(const Node& p_nd, const Node& p_nd2) const {
            return (this->getBindingFor (p_nd,p_nd2) != NULL);
        }

        const Link* RuleSet::bind(const Node& p_nd, const Node& p_nd2) const {
            const Binding* l_bnd = this->getBindingFor (p_nd,p_nd2);
            if (l_bnd)
                return l_bnd->bind (p_nd,p_nd2);
            else
                return NULL;
        }

        const string RuleSet::locale() const {
            return m_dom->documentElement ().attribute("locale").toStdString();
        }

        Parser::Parser() : m_lcl ( string() ) { }

        Parser::Parser ( const string& p_lcl ) : m_lcl ( p_lcl ) { }

        const string Parser::locale () const { return m_lcl; }

        void Parser::setLocale ( const string& p_lcl ) { m_lcl = p_lcl; }

        StringVector Parser::getTokens ( const string& p_str ) {
            tokenizer<> tkn ( p_str );
            StringVector l_theTokens;

            for ( tokenizer<>::const_iterator itr = tkn.begin (); itr != tkn.end (); ++itr ) {
                const QString l_str ( ( *itr ).c_str() );
                l_theTokens.push_back ( l_str.toLower ().toStdString () );
            }

            return l_theTokens;
        }

        /// @todo Wrap the ID obtaining method into a private method by the parser.
        NodeVector Parser::formNodes ( StringVector& l_tokens ) {
            NodeVector l_theNodes;

            for ( StringVector::const_iterator itr = l_tokens.begin (); itr != l_tokens.end (); ++itr ) {
                const string l_curToken = *itr, l_theID = md5 ( l_curToken );
                const Node* l_theNode = Node::obtain ( m_lcl, l_theID );
                const bool l_ndExsts = Node::exists ( m_lcl, l_theID );

                if ( l_ndExsts )
                    l_theNodes.push_back ( *l_theNode );
                else
                    l_theNodes.push_back ( * ( Node::buildPseudo ( m_lcl, l_theID, l_curToken ) ) );
            }

            return l_theNodes;
        }

        // Salvaged this method's algorithm from an older version of the parser.
        NodeTree Parser::expandNodes ( NodeTree& p_tree, const int& p_size, const int& p_level ) {
            if ( p_level == p_tree.size () )
                return ( NodeTree() );

            //cout << "(ling) [Parser] Level " << p_level << " should generate " << p_size << " paths." << endl;
            const NodeVector l_curBranch = p_tree.at ( p_level );
            const bool isAtEnd = ( p_level + 1 == p_tree.size () );

            if ( l_curBranch.empty () ) {
                //cout << "(ling) [Parser] WARNING! Invalid level detected at level " << p_level << "." << endl;
                return ( NodeTree() );
            }

            //cout << "(ling) [Parser] Level " << p_level << " has " << l_curBranch.size() << " variations." << endl;

            const int l_mxSize = p_size / l_curBranch.size ( );

            NodeTree l_chldBranches, l_foundStems = expandNodes ( p_tree , l_mxSize , p_level + 1 );

            //cout << "(ling) [Parser] Level " << p_level << " expects " << l_foundStems.size() * l_curBranch.size () << " paths." << endl;
            for ( NodeVector::const_iterator jtr = l_curBranch.begin ( ); jtr != l_curBranch.end ( ); jtr ++ ) {
                const Node l_curLvlNd = * jtr;

                if ( !isAtEnd ) {
                    for ( NodeTree::iterator itr = l_foundStems.begin ( ); itr != l_foundStems.end ( ); itr ++ ) {
                        NodeVector tmpVector, // creates the current vector (1 of x, x = l_curBranch.size();
                        theVector = * itr;
                        tmpVector.push_back ( l_curLvlNd );
                        tmpVector.insert ( tmpVector.end ( ), theVector.begin ( ), theVector.end ( ) );
                        l_chldBranches.push_back ( tmpVector ); // add this current branch to list.
                    }
                } else { // the end of the line!
                    NodeVector tmpVector;
                    tmpVector.push_back ( l_curLvlNd );
                    l_chldBranches.push_back ( tmpVector ); // add this current branch to list.
                }
            }

            //cout << "(ling) [Parser] Level " << p_level << " generated " << l_chldBranches.size() << " branches." << endl;
            return l_chldBranches;
        }

        NodeTree Parser::expandNodes ( NodeVector& p_nodVtr ) {
            int l_totalPaths = 1;
            NodeTree l_metaTree;

            for ( NodeVector::const_iterator itr = p_nodVtr.begin (); itr != p_nodVtr.end (); itr++ ) {
                const Node l_theNode = *itr;
                const Leximap* l_lxmp = l_theNode.flags();
                NodeVector l_variations = FlatNode::expand ( l_theNode );
                const unsigned int size = l_variations.size ();

                if ( itr != p_nodVtr.begin() )
                    l_totalPaths *= size;

                l_metaTree.push_back ( l_variations );
            }

            //cout << "(ling) [Parser] Expanding across " << p_nodVtr.size () << " levels and expecting " << l_totalPaths << " different paths..." << endl;
            NodeTree l_tree = expandNodes ( l_metaTree , l_totalPaths , 0 );

            //cout << "(ling) [Parser] Found " << l_tree.size() << " paths." << endl;

            return l_tree;
        }

        /// @todo Determine a means of generating unique signatures.
        const string Parser::formShorthand ( const NodeVector& p_ndVtr, const Node::FormatDensity& p_sigVerb ) {
            string l_ndShrthnd;

            for ( NodeVector::const_iterator itr = p_ndVtr.begin (); itr != p_ndVtr.end (); ++itr ) {
                const FlatNode l_nd = *itr;
                l_ndShrthnd += l_nd.toString ( p_sigVerb );
            }

            return l_ndShrthnd;
        }

        /// @todo Question user to discern which branch is the branch that should be solidified.
        void Parser::parse ( const string& p_txt ) {
            process(p_txt);
        }

        /// @todo Convert a collection of nodes into an syntax that's interpretable by the system.
        /// @todo Find a means of converting this object into an object that represents ontological formats.
        void Parser::process ( const string& p_txt ) {
            StringVector l_tokens = getTokens ( p_txt );
            NodeVector l_theNodes = formNodes ( l_tokens );
            NodeTree l_nodeTree = expandNodes ( l_theNodes );

            MeaningVector l_meaningVtr;
            for (NodeTree::const_iterator itr = l_nodeTree.begin (); itr != l_nodeTree.end (); itr++) {
                const NodeVector l_ndVtr = *itr;
                //cout << "(ling) [Parser] " << "Forming meaning #" << (l_meaningVtr.size () + 1) << "..." << endl;
                const Meaning* l_meaning = Meaning::form(l_ndVtr);

                if (l_meaning != NULL)
                    l_meaningVtr.push_back (l_meaning);
            }

            unique(l_meaningVtr.begin(),l_meaningVtr.end ());
            cout << "(ling) [Parser] " << l_nodeTree.size () << " paths formed " << l_meaningVtr.size () << " meanings." << endl;

            for (MeaningVector::const_iterator itr2 = l_meaningVtr.begin (); itr2 != l_meaningVtr.end (); itr2++){
                const Meaning* l_mngItr = *itr2;
                cout << l_mngItr->toText () << endl;
            }
        }

        Meaning::Meaning() : m_lnk(NULL), m_lnkVtr(NULL) { }

        Meaning::Meaning(const Link* p_lnk, const LinkVector* p_lnkVtr) : m_lnk(p_lnk), m_lnkVtr(p_lnkVtr) {
            //cout << "(ling) [Meaning] Formed a Meaning with one primary link and " << p_lnkVtr->size () << " total links." << endl;
        }

        const Meaning* Meaning::form(const Link* p_lnk, const LinkVector* p_lnkVtr) {
            return new Meaning(p_lnk,p_lnkVtr);
        }

        /// @todo Raise an exception and crash if this word is foreign OR have it be registered under a psuedo word OR implement a means of creating a new RuleSet.
        /// @todo If a word fails the test, crash and explain there's a grammatical mistake OR add another rule if under educational mode.
        const Meaning* Meaning::form(const NodeVector& p_ndVtr, LinkVector* p_lnkVtr) {
            NodeVector::const_iterator l_ndItr = p_ndVtr.begin (), l_ndItrEnd = p_ndVtr.end ();
            NodeVector l_ndVtr;
            int i = 0;

            for ( ; l_ndItr != p_ndVtr.end (); l_ndItr++, i++) {
                cout << i << endl;
                const Node *l_nd, *l_nd2;
                if (p_ndVtr.size () == 2){
                    l_nd = &p_ndVtr.front ();
                    l_nd2 = &p_ndVtr.back ();
                }
                else {
                    if (l_ndItr == p_ndVtr.end ())
                        break;
                    else {
                        l_nd = &(*(l_ndItr));
                        l_nd2 = &(*(l_ndItr + 1));
                    }
                }

                const Rule* l_rl = Rule::obtain(*l_nd);

                if (p_ndVtr.size () == 2)
                    cout << l_nd->symbol () << " & " << l_nd2->symbol () << endl;

                if (l_rl){
                    if (!l_rl->canBind (*l_nd,*l_nd2))
                        l_ndVtr.push_back (*l_nd); // Try it again next time,
                    else {
                        const Link* l_lnk = l_rl->bind(*l_nd,*l_nd2);
                        p_lnkVtr->push_back (l_lnk);
                        l_ndItr++; // Move on to the next word.
                        l_ndVtr.push_back (static_cast<Node>(*l_lnk->source ()));
                    }
                }
            }

            cout << "(ling) [Meaning] Formed " << p_lnkVtr->size () << " links with " << l_ndVtr.size () << " nodes left to parse." << endl;

            if (!p_lnkVtr->empty ()){
                if (!(p_lnkVtr->size () >= 1 && l_ndVtr.size () == 1))
                    return Meaning::form (l_ndVtr , p_lnkVtr);
                else {
                    cout << "(ling) [Meaning] Crafted a Meaning of " << p_lnkVtr->size () << " links." << endl;
                    return new Meaning(p_lnkVtr->back () , p_lnkVtr);
                }
            }
            else
                return NULL;
        }

        const Link* Meaning::base () const { return m_lnk; }
        const LinkVector* Meaning::siblings () const { return m_lnkVtr; }

        const string Meaning::toText () const {
            cout << m_lnkVtr->size () << " links." << endl;
            for (LinkVector::const_iterator i = m_lnkVtr->begin (); i != m_lnkVtr->end (); i++){
                const Link* l_lnk = *i;
                cout << l_lnk->source ()->symbol () << " -> " << l_lnk->destination ()->symbol ()  << endl;
            }

            return " ";
        }
    }
}

// kate: indent-mode cstyle; space-indent on; indent-width 4;
