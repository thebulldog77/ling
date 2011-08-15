/**
 * @file    syntax.cpp
 * @author  Jacky Alcine <jackyalcine@gmail.com>
 * @date February 4, 2011, 12:07 AM
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

#include "syntax.hpp"
#include <boost/tokenizer.hpp>

using namespace boost;

using boost::tokenizer;

namespace Wintermute {
    namespace Linguistics {
        Node::Node() : m_lxdt ( NULL ) { }

        Node::Node ( Lexidata* p_lxdt ) : m_lxdt ( p_lxdt ) {}

        const string Node::id () const {
            return m_lxdt->id();
        }

        const string Node::locale() const {
            return m_lxdt->locale();
        }

        const string Node::symbol() const {
            return m_lxdt->symbol ();
        }

        const Leximap* Node::flags() const {
            return m_lxdt->flags ();
        }

        const string Node::toString ( const FormatDensity& p_density ) const {
            Leximap::const_iterator l_ndFlg = flags ()->begin();
            string sig;
            switch ( p_density ) {
            case MINIMAL:
                sig = ( *l_ndFlg ).second.at ( 0 );
                break;

            case EXTRA:
                sig = ( *l_ndFlg ).second;
                break;

            default:
            case FULL:
                sig = ( *l_ndFlg ).second + string ( "[" ) + ( *l_ndFlg ).first + string ( "]" );
                break;
            }

            return sig;
        }

        const string Node::toString ( const Node* p_nd, const FormatDensity& p_density ) {
            return p_nd->toString ( p_density );
        }

        const string Node::toString ( const NodeVector& p_ndVtr, const FormatDensity& p_density ) {
            string sig;

            for ( NodeVector::const_iterator itr = p_ndVtr.begin (); itr != p_ndVtr.end (); itr++ ) {
                sig += ( *itr )->toString ( p_density );

                if ( ( itr + 1 ) != p_ndVtr.end () )
                    sig += ">";
            }
        }

        const Node* Node::obtain ( const string& p_lcl, const string& p_id ) {
            Lexidata l_lexdata ( p_id , p_lcl );

            if ( exists ( p_lcl , p_id ) ) {
                LocalStorage* l_strg = dynamic_cast<LocalStorage*> ( LocalStorage::obtain ( l_lexdata ) );
                l_strg->load ();
                Lexidata* l_datum = l_strg->LocalLoadModel::lexicalData ();
                return new Node ( l_datum );
            } else
                return NULL;
        }

        const Node* Node::buildPseudo ( const string& p_lcl, const string& p_id, const string& p_sym ) {
            Leximap l_map;
            l_map.insert ( Leximap::value_type ( "-1", "Bz" ) );
            return new Node ( ( new Lexidata ( p_id, p_lcl, p_sym, l_map ) ) );
        }

        const bool Node::exists ( const string& p_lcl, const string& p_id ) {
            Lexidata l_lexdata ( p_id , p_lcl );
            return Storage::exists ( l_lexdata );
        }

        Node::~Node () {}

        FlatNode::FlatNode() : Node() { }

        FlatNode::FlatNode ( const string& m_id, const string& m_lcl, const string& m_sym, const StringCollection::value_type& m_pair ) {
            Leximap l_map;
            l_map.insert ( m_pair );
            ::Node ( ( new Lexidata ( m_id , m_lcl , m_sym , l_map ) ) );
        }

        /// @todo Actually add the approriate flag by the specified index.
        FlatNode::FlatNode ( const Node* p_nod, const int& p_indx ) {
            Leximap l_map;
            l_map.insert ( * ( p_nod->flags()->begin ()) );
            ::Node ( ( new Lexidata ( p_nod->id(),
                                      p_nod->locale(),
                                      p_nod->symbol(),
                                      l_map ) ) );
        }

        const FlatNode* FlatNode::form ( const string& m_id, const string& m_lcl, const string& m_sym, const StringCollection::value_type& m_pair ) {
            return new FlatNode ( m_id , m_lcl , m_sym , m_pair );
        }

        const FlatNode* FlatNode::form ( const Node* p_nod, const int& p_indx ) {
            return new FlatNode ( p_nod,p_indx );
        }

        NodeVector FlatNode::expand ( const Node* p_nod ) {
            NodeVector l_vtr;
            const Leximap* l_map = p_nod->flags ();

            for ( Leximap::const_iterator itr = l_map->begin (); itr != l_map->end (); itr++ ) {
                Leximap l_lxmp;
                l_lxmp.insert ( *itr );
                //cout << "(ling) [FlatNode] " << (*itr).first << " " << (*itr).second << endl;
                Lexidata* l_lxdt = new Lexidata ( p_nod->id (), p_nod->locale (), p_nod->symbol (), l_lxmp );
                l_vtr.push_back ( (new Node(l_lxdt)) );
            }

            //cout << "(ling) [FlatNode] Expanded symbol '" << p_nod.symbol () << "' to spread across its " << l_map->size() << " variations." << endl;

            return l_vtr;
        }

        const char FlatNode::type() const {
            const Leximap::const_iterator itr = m_lxdt->flags ()->begin ();
            return ( ( *itr ).second ) [0];
        }

        FlatNode::~FlatNode() { }

        Link::Link() : m_flgs ( string() ) { }

        Link::Link ( const FlatNode* p_src, const FlatNode* p_dst, const string& p_flgs, const string& p_lcl ) : m_src ( p_src ),
                m_dst ( p_dst ), m_flgs ( p_flgs ), m_lcl ( p_lcl ) { }

        const FlatNode* Link::source() const {
            return m_src;
        }

        const FlatNode* Link::destination() const {
            return m_dst;
        }

        const string Link::flags () const {
            return m_flgs;
        }

        const string Link::locale() const {
            return m_lcl;
        }

        const string Link::toString() const {
            return m_src->toString() + "," + m_src->id() + ":" +
                   m_dst->toString() + "," + m_dst->id() + ":" +
                   m_flgs + ":" +
                   m_lcl;
        }

        const Link* Link::form ( const FlatNode * p_src, const FlatNode * p_dst, const string & p_flgs, const string & p_lcl ) {
            return new Link ( p_src , p_dst , p_flgs , p_lcl );
        }

        const Link* Link::fromString ( const string& p_data ) {
            char_separator<char> l_frstLvl ( ":" );
            char_separator<char> l_scndLvl ( "," );

            typedef tokenizer<char_separator<char> > Tokenizer;
            Tokenizer toks ( p_data,l_frstLvl );

            Tokenizer::const_iterator itr = toks.begin();
            const string node1 = *itr;
            const string node2 = * ( itr++ );
            const string flags = * ( itr++ );
            const string lcl = node1;

            Tokenizer toks2 ( node1,l_scndLvl );
            const string node1_id = * ( ++ ( toks2.begin() ) );

            Tokenizer toks3 ( node2,l_scndLvl );
            const string node2_id = * ( ++ ( toks3.begin() ) );

            return new Link ( static_cast<const FlatNode*> ( Node::obtain ( lcl,node1_id ) ),
                              static_cast<const FlatNode*> ( Node::obtain ( lcl,node2_id ) ),
                              flags,lcl );
        }
    }
}
// kate: indent-mode cstyle; space-indent on; indent-width 4;
