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
#include <exception>
#include <boost/smart_ptr.hpp>
#include <boost/tokenizer.hpp>

using namespace boost;

using boost::tokenizer;
using Wintermute::Data::Linguistics::Lexical::LoadModel;
using Wintermute::Data::Linguistics::Lexical::SaveModel;
using Wintermute::Data::Linguistics::Lexical::Storage;

namespace Wintermute {
    namespace Linguistics {

        const string Node::toString ( const FormatDensity& p_density ) const {
            Leximap::const_iterator l_flgItr = m_lxdt.flags ().begin ();
            string sig;
            switch ( p_density ) {
            case MINIMAL:
                sig = (*l_flgItr).first.at ( 0 );
                break;

            case EXTRA:
                sig = (*l_flgItr).second;
                break;

            default:
            case FULL:
                sig = (*l_flgItr).second + string ( "[" ) +
                      (*l_flgItr).first + string ( "]" );
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

        const Node* Node::create( const Lexidata* p_lxdt ){
            SaveModel* l_svmdl = Storage::obtain (p_lxdt);
            l_svmdl->save ();
            return Node::obtain (*(p_lxdt->locale ()),*(p_lxdt->id ()));
        }

        /// @bug Something funky is happening here.
        const Node* Node::obtain ( const string& p_lcl, const string& p_id ) {
            Lexidata* l_lexdata = new Lexidata( &p_id , &p_lcl );

            if ( exists ( p_lcl , p_id ) ) {
                LoadModel* l_ldmdl = Storage::obtain(l_lexdata);
                Lexidata* l_lxdt = l_ldmdl->load ();
                return new Node ( *l_lxdt );
            } else
                return NULL;
        }

        const Node* Node::buildPseudo ( const string& p_id, const string& p_lcl, const string& p_sym ) {
            Leximap l_map;
            l_map.insert ( Leximap::value_type("-1", "Bz" ) );
            return new Node ( ( Lexidata ( &p_id, &p_lcl, &p_sym, l_map ) ) );
        }

        const bool Node::exists ( const string& p_lcl, const string& p_id ) {
            return Storage::exists ( ( new Lexidata( &p_id , &p_lcl ) ) );
        }

        FlatNode::FlatNode() : Node() { }

        FlatNode::FlatNode ( const string& m_id, const string& m_lcl, const string& m_sym, const Leximap::value_type& m_pair ) {
            Leximap l_map;
            l_map.insert(m_pair);
            ::Node ( ( new Lexidata ( &m_id , &m_lcl , &m_sym , l_map ) ) );
        }

        /// @todo Actually add the approriate flag by the specified index.
        FlatNode::FlatNode ( const Node* p_nod, const int& p_indx ) {
            Leximap l_map = p_nod->flags ();
            Leximap::const_iterator itr = l_map.begin ();
            //for (int i = 0; i < p_indx; i++){ itr++ }
            l_map.insert ( Leximap::value_type((*itr).first, (*itr).second) );
            ::Node ( ( new Lexidata ( p_nod->id(),
                                      p_nod->locale(),
                                      p_nod->symbol(),
                                      l_map ) ) );
        }

        const FlatNode* FlatNode::form ( const string& m_id, const string& m_lcl, const string& m_sym, const Leximap::value_type& m_pair ) {
            return new FlatNode ( m_id , m_lcl , m_sym , m_pair );
        }

        const FlatNode* FlatNode::form ( const Node* p_nd, const int& p_indx ) {
            return new FlatNode ( p_nd,p_indx );
        }

        NodeVector FlatNode::expand ( const Node* p_nd ) {
            NodeVector l_vtr;
            Leximap l_map;
            int l_indx = 0;

            l_map = p_nd->flags ();

            for ( Leximap::iterator itr = l_map.begin (); itr != l_map.end (); l_indx++, itr++ )
                l_vtr.push_back ( const_cast<FlatNode*>(FlatNode::form(p_nd->id (),p_nd->locale (),p_nd->symbol (),*itr)) );

            //qDebug() << "(ling) [FlatNode] Expanded symbol" << p_nd->symbol ().c_str () << "to spread across its" << l_map.size() << "variations.";

            return l_vtr;
        }

        const char FlatNode::type() const {
            const Leximap::const_iterator itr = m_lxdt.flags ().begin ();
            return (*itr).second [0];
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

        QDebug operator<<(QDebug dbg, const NodeVector &p_ndVtr) {
            foreach(const Node* p_nd, p_ndVtr)
               dbg.space () << p_nd << ",";

             return dbg.space();
        }

        QDebug operator<<(QDebug dbg, const LinkVector &p_lnkVtr){
            foreach(const Link* p_lnk, p_lnkVtr)
                dbg.space () << p_lnk;

            return dbg.space ();
        }

    }
}
// kate: indent-mode cstyle; space-indent on; indent-width 4;
