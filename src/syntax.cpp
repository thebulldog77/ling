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
#include <QtDebug>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <boost/smart_ptr.hpp>
#include <boost/tokenizer.hpp>
#include <data/config.hpp>

using namespace boost;

using boost::tokenizer;
using Wintermute::Data::Linguistics::Lexical::LoadModel;
using Wintermute::Data::Linguistics::Lexical::SaveModel;
using Wintermute::Data::Linguistics::Lexical::Storage;

namespace Wintermute {
    namespace Linguistics {

        const QString Node::toString ( const Node::FormatVerbosity& p_density ) const {
            Lexical::FlagMapping::ConstIterator l_flgItr = m_lxdt.flags ().begin ();
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

            return sig;
        }

        const QString Node::toString ( const Node* p_nd, const FormatVerbosity& p_density ) {
            return p_nd->toString ( p_density );
        }

        const QString Node::toString ( const NodeList& p_ndVtr, const FormatVerbosity& p_density ) {
            QString sig;

            for ( NodeList::const_iterator itr = p_ndVtr.begin (); itr != p_ndVtr.end (); itr++ ) {
                sig += ( *itr )->toString ( p_density );

                if ( ( itr + 1 ) != p_ndVtr.end () )
                    sig += ">";
            }
        }

        Node* Node::create( const Lexical::Data& p_lxdt ){
            QDBusMessage l_call = QDBusMessage::createMethodCall ("org.thesii.Wintermute.Data","/Nodes","org.thesii.Wintermute.Data.NodeAdaptor","write");
            l_call << QVariant::fromValue(p_lxdt);
            qDebug() << l_call;
            QDBusMessage l_reply = QDBusConnection::sessionBus ().call(l_call,QDBus::BlockWithGui);

            if (l_reply.type () == QDBusMessage::ReplyMessage){
                const Lexical::Data l_lxdt = l_reply.arguments ().at (0).value<Lexical::Data>();
                Lexical::Cache::write (l_lxdt);
                qDebug() << QVariant::fromValue<Lexical::Data>(l_lxdt);
                return Node::obtain ( l_lxdt.locale (), l_lxdt.id () );
            } else if (l_reply.type () == QDBusMessage::ErrorMessage) {
                qDebug() << "(ling) [Node] Error creaing Node data over D-Bus."
                         << l_reply.errorMessage ();
                return NULL;
            }

            return NULL;
        }

        Node* Node::obtain ( const QString& p_lcl, const QString& p_id ) {
            Lexical::Data l_dt( p_id , p_lcl );

            if ( exists ( p_lcl , p_id ) ) {
                QDBusMessage l_call = QDBusMessage::createMethodCall ("org.thesii.Wintermute.Data","/Nodes","org.thesii.Wintermute.Data.NodeAdaptor","read");
                l_call << QVariant::fromValue(l_dt);
                qDebug() << l_call;
                QDBusMessage l_reply = QDBusConnection::sessionBus ().call(l_call,QDBus::BlockWithGui);
                if (l_reply.type () == QDBusMessage::ReplyMessage){
                    l_dt = l_reply.arguments ().at (0).value<Lexical::Data>();
                    qDebug() << QVariant::fromValue<Lexical::Data>(l_dt);
                    return new Node ( l_dt );
                } else if (l_reply.type () == QDBusMessage::ErrorMessage) {
                    qDebug() << "(ling) [Node] Error obtaining Node data from over D-Bus."
                             << l_reply.errorMessage ();
                }
            }

            return NULL;
        }

        Node* Node::buildPseudo ( const QString& p_lcl, const QString& p_sym ) {
            Lexical::Data l_dt("",p_lcl,p_sym);
            QDBusMessage l_call = QDBusMessage::createMethodCall ("org.thesii.Wintermute.Data","/Nodes","org.thesii.Wintermute.Data.NodeAdaptor","pseudo");
            l_call << QVariant::fromValue(l_dt);
            qDebug() << l_call;
            QDBusMessage l_reply = QDBusConnection::sessionBus ().call(l_call,QDBus::BlockWithGui);

            if (l_reply.type () == QDBusMessage::ErrorMessage){
                qDebug() << "(data) [Node] Unable to obtain a psuedo node for the" << p_lcl
                         << "locale of the word" << p_sym << "."
                         <<  l_reply.errorMessage ();
                return NULL;
            } else if (l_reply.type () == QDBusMessage::ReplyMessage){
                l_dt = l_reply.arguments ().at (0).value<Lexical::Data>();
                qDebug() << QVariant::fromValue<Lexical::Data>(l_dt);
            }

            return new Node ( l_dt );
        }

        const bool Node::exists ( const QString& p_lcl, const QString& p_id ) {
            qDBusRegisterMetaType<Lexical::Data>();
            Lexical::Data l_dt(p_id,p_lcl);
            QVariant l_vrnt = QVariant::fromValue(l_dt);
            QDBusMessage l_call = QDBusMessage::createMethodCall (WNTRDATA_DBUS_SERVICE,"/Nodes","org.thesii.Wintermute.Data.NodeAdaptor","exists");
            l_call << l_vrnt;
            qDebug() << "(ling) [Node] <exists>" << p_lcl;
            QDBusMessage l_reply = QDBusConnection::sessionBus ().call(l_call,QDBus::BlockWithGui);

            if (l_reply.type () == QDBusMessage::ErrorMessage){
                qDebug() << "(data) [Node] Unable to determine existance of" << p_id << p_lcl << ":"
                         << l_reply.errorMessage ();
            } else if (l_reply.type () == QDBusMessage::ReplyMessage){
                qDebug() << l_reply;
                //const bool l_rlpy = l_reply
                //qDebug() << "(ling) [Node] Exists:"<< l_rlpy;
                //return l_rlpy;
            }

            return false;
        }

        Node* Node::form ( const Lexical::Data l_dt ) {
            return new Node ( l_dt );
        }

        NodeList Node::expand ( const Node* p_nd ) {
            NodeList l_vtr;
            Lexical::FlagMapping l_map;
            int l_indx = 0;

            l_map = p_nd->flags ();

            for ( Lexical::FlagMapping::iterator itr = l_map.begin (); itr != l_map.end (); l_indx++, itr++ ){
                Lexical::FlagMapping l_mp;
                l_mp.insert (itr.key (),itr.value ());
                Lexical::Data l_dt(p_nd->id (),p_nd->locale (), p_nd->symbol ());
                l_dt.setFlags (l_mp);
                l_vtr.push_back ( const_cast<Node*>(Node::form(l_dt)) );
            }

            qDebug() << "(ling) [Node] Expanded symbol" << p_nd->symbol () << "to spread across its" << l_map.size() << "variations.";

            return l_vtr;
        }

        /// @todo Create a serialized version of the Link.
        const QString Link::toString() const {
            return QString::null;
        }

        Link* Link::form ( const Node * p_src, const Node * p_dst, const QString & p_flgs, const QString & p_lcl ) {
            return new Link ( p_src , p_dst , p_flgs , p_lcl );
        }

        Link* Link::fromString ( const QString& p_data ) {
            char_separator<char> l_frstLvl ( ":" );
            char_separator<char> l_scndLvl ( "," );

            typedef tokenizer<char_separator<char> > Tokenizer;
            Tokenizer toks ( p_data.toStdString() ,l_frstLvl );

            Tokenizer::const_iterator itr = toks.begin();
            const std::string node1 = *itr,
                              node2 = * ( itr++ );

            Tokenizer toks2 ( node1,l_scndLvl );
            Tokenizer toks3 ( node2,l_scndLvl );
            
            const QString node1_id = QString::fromStdString(* ( ++ ( toks2.begin() ) )),
                          node2_id = QString::fromStdString(* ( ++ ( toks3.begin() ) )),
                          flags    = QString::fromStdString(* ( itr++ )),
                          lcl      = QString::fromStdString(node1);

            return new Link ( Node::obtain ( lcl ,node1_id ), Node::obtain ( lcl,node2_id ),
                              flags, lcl );
        }

        QDebug operator<<(QDebug dbg, const Node* p_nd) {
             dbg.nospace () << "[" << p_nd->symbol () << " (" << p_nd->toString (Node::EXTRA) << "):" << p_nd->locale ().toStdString ().c_str () << "]";
             return dbg.space();
        }

        QDebug operator<<(QDebug dbg, const Link* p_lnk) {
             dbg.nospace () << "[Level " << p_lnk->level() << "](type:" << p_lnk->flags () << ")"
                            << p_lnk->source () << "->" << p_lnk->destination ();
             return dbg.space();
        }
    }
}
// kate: indent-mode cstyle; space-indent on; indent-width 4;
