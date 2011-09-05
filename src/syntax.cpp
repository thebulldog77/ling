/**
 * @file    syntax.cpp
 * @author  Wintermute Developers <wintermute-devel@lists.launchpad.net>
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

        const string Node::toString ( const Node::FormatVerbosity& p_density ) const {
            Lexical::DataFlagMap::ConstIterator l_flgItr = m_lxdt.flags ().begin ();
            QString sig;
            switch ( p_density ) {
            case MINIMAL:
                sig = l_flgItr.value ().at(0);
                break;

            case EXTRA:
                sig = l_flgItr.value ();
                break;

            default:
            case FULL:
                sig = l_flgItr.value ();
                sig += "[";
                sig += l_flgItr.key();
                sig += "]";
                break;
            }

            return sig.toStdString ();
        }

        const string Node::toString ( const Node* p_nd, const FormatVerbosity& p_density ) {
            return p_nd->toString ( p_density );
        }

        const string Node::toString ( const NodeList& p_ndVtr, const FormatVerbosity& p_density ) {
            string sig;

            for ( NodeList::const_iterator itr = p_ndVtr.begin (); itr != p_ndVtr.end (); itr++ ) {
                sig += ( *itr )->toString ( p_density );

                if ( ( itr + 1 ) != p_ndVtr.end () )
                    sig += ">";
            }
        }

        const Node* Node::create( const Lexical::Data& p_lxdt ){
            Lexical::Cache::write (p_lxdt);
            return Node::obtain ( p_lxdt.locale ().toStdString (), p_lxdt.id ().toStdString () );
        }

        /// @bug Something funky is happening here.
        const Node* Node::obtain ( const string& p_lcl, const string& p_id ) {
            Lexical::Data l_dt = Lexical::Data::createData ( QString::fromStdString (p_id) , QString::fromStdString (p_lcl) );

            if ( exists ( p_lcl , p_id ) ) {
                if (Lexical::Cache::read(l_dt))
                    return new Node ( l_dt );
            }

            return NULL;
        }

        const Node* Node::buildPseudo ( const string& p_lcl, const string& p_sym ) {
            Lexical::Data l_dt = Lexical::Data::createData (QString::fromStdString (""),QString::fromStdString (p_lcl),QString::fromStdString(p_sym));
            Lexical::Cache::pseudo (l_dt);
            return new Node ( l_dt );
        }

        const bool Node::exists ( const string& p_lcl, const string& p_id ) {
            Lexical::Data l_dt = Lexical::Data::createData ( QString::fromStdString (p_id) , QString::fromStdString (p_lcl) );
            return Lexical::Cache::exists(l_dt);
        }

        const Node* Node::form ( const Lexical::Data l_dt ) {
            return new Node ( l_dt );
        }

        NodeList Node::expand ( const Node* p_nd ) {
            NodeList l_vtr;
            Lexical::DataFlagMap l_map;
            int l_indx = 0;

            l_map = p_nd->flags ();

            for ( Lexical::DataFlagMap::iterator itr = l_map.begin (); itr != l_map.end (); l_indx++, itr++ ){
                Lexical::DataFlagMap l_mp;
                l_mp.insert (itr.key (),itr.value ());
                Lexical::Data l_dt = Lexical::Data::createData (p_nd->id (),p_nd->locale (), p_nd->symbol ());
                l_dt.setFlags (l_mp);

                l_vtr.push_back ( const_cast<Node*>(Node::form(l_dt)) );
            }

            //qDebug() << "(ling) [Node] Expanded symbol" << p_nd->symbol ().c_str () << "to spread across its" << l_map.size() << "variations.";

            return l_vtr;
        }

        const string Link::toString() const {
            return m_src->toString() + "," + m_src->id().toStdString () + ":" +
                   m_dst->toString() + "," + m_dst->id().toStdString () + ":" +
                   m_flgs + ":" + m_lcl;
        }

        const Link* Link::form ( const Node * p_src, const Node * p_dst, const string & p_flgs, const string & p_lcl ) {
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

            return new Link ( Node::obtain ( lcl,node1_id ), Node::obtain ( lcl,node2_id ),
                              flags, lcl );
        }

        QDebug operator<<(QDebug dbg, const Node* p_nd) {
             dbg.nospace () << "[" << p_nd->symbol () << " (" << QString::fromStdString (p_nd->toString (Node::EXTRA)) << "):" << p_nd->locale ().toStdString ().c_str ()<< "]";
             return dbg.space();
        }

        QDebug operator<<(QDebug dbg, const Link* p_lnk) {
             dbg.nospace () << "[Level " << p_lnk->level() << "](type:" << QString::fromStdString (p_lnk->flags ()) << ")"
                            << p_lnk->source () << "->" << p_lnk->destination ();
             return dbg.space();
        }
    }
}
// kate: indent-mode cstyle; space-indent on; indent-width 4;
