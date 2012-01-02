/**
 * @file    meanings.cpp
 * @author  Wintermute Developers <wintermute-devel@lists.launchpad.net>
 * @date    August 20, 2011 8:58 PM
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

#include <iomanip>
#include <iostream>
#include "syntax.hpp"
#include "parser.hpp"
#include "meanings.hpp"
#include <plugins/data/ontology.hpp>

using namespace std;
using namespace Wintermute::Linguistics;

namespace Wintermute {
    namespace Linguistics {
        int Meaning::s_cnt = 0;
        Meaning::Meaning() { }

        Meaning::Meaning(const LinkList &p_lnkVtr) : m_lnkVtr(p_lnkVtr) {
            __init();
        }

        Meaning::Meaning(const Meaning &p_meaning) : m_lnkVtr(p_meaning.m_lnkVtr) {
            __init();
        }

        void Meaning::__init() {
            m_ontoMap.clear();

            foreach (Link* l_lnk, m_lnkVtr) {
                Node* l_nd = const_cast<Node*>(l_lnk->source ());
                m_ontoMap.insert(l_nd,l_lnk);
            }

            qDebug() << "(ling) [Meaning] Encapsulates" << m_ontoMap.uniqueKeys ();
        }

        /// @todo Should a pointer to the end iterator be passed? It'd reduce the need to constantly call the method but increases the size of the signature of the mention.
        void Meaning::alignNodes(const NodeList *p_lstAll, const Node *p_ndLeft, const Node *p_ndRight, NodeList* p_lstParsed, NodeList::ConstIterator* p_ndItr){
            const NodeList::ConstIterator l_ndItrEnd = p_lstAll->end ();
            if ( p_lstAll->size () == 2 ) {
                p_ndLeft = p_lstAll->front ();
                p_ndRight = p_lstAll->back ();
            } else {
                if ( ( *p_ndItr + 1 ) != l_ndItrEnd ) {
                    p_ndLeft =  *( * ( p_ndItr ) );
                    p_ndRight = *( * ( p_ndItr + 1 ) );
                } else {
                    if (p_lstParsed->size () == 1)
                        p_lstParsed->push_back(**p_ndItr);
                }
            }
        }

        const Meaning* Meaning::form ( LinkList* p_lnkLst, const NodeList& p_ndVtr ) {
            Meaning::s_cnt++;
            if (p_lnkLst == NULL)
                p_lnkLst = new LinkList;

            cout << endl << setw(6) << setfill('=') << '=';
            cout << " Level ";
            cout << setw(4) << setfill('0') << right << s_cnt << ' ';
            cout << setw(6) << setfill('=') << '=' << endl;

            NodeList::ConstIterator l_ndItr = p_ndVtr.begin ();
            NodeList l_ndLst;
            QStringList* l_hideList = NULL;
            bool l_hideOther = false, l_hideThis = false;

            if (p_ndVtr.size () != 1) {                
                const Node *l_ndLeft;
                const Node *l_ndRight;
                const NodeList::ConstIterator l_ndItrEnd = p_ndVtr.end ();

                for ( ; l_ndItr != l_ndItrEnd; l_ndItr++ ) {
                    alignNodes(&p_ndVtr,l_ndLeft,l_ndRight,&l_ndLst,&l_ndItr);

                    if (l_hideList) {
                        const QString l_k = l_ndLeft->toString (Node::EXTRA);
                        bool l_b = false;
                        foreach (const QString l_s, *l_hideList)
                            if (l_k.contains (l_s)) l_b = true;

                        if (!l_b) {
                            //qDebug() << "(ling) [Meaning] *** This node broke the filter; may appear on next round." << endl;
                            l_hideList = NULL;
                            l_hideThis = false;
                        }
                        else {
                            l_hideThis = true;
                            //qDebug() << "(ling) [Meaning] *** Node won't appear in next round due to filter '" << l_hideList->join (",") << "'." << endl;
                        }
                    }

                    qDebug() << "(ling) [Meaning] Current node: " << l_ndLeft;

                    const Binding* l_bnd = Binding::obtain ( *l_ndLeft,*l_ndRight );
                    const Link* l_lnk;
                    if ( l_bnd ) {
                        l_lnk = l_bnd->bind ( *l_ndLeft,*l_ndRight );
                        l_lnk->m_lvl = Meaning::s_cnt;
                        p_lnkLst->push_back ( const_cast<Link*>(l_lnk) );

                        QString l_hide = l_bnd->getAttrValue("hide");
                        QString l_hideNext = l_bnd->getAttrValue("hideNext");
                        QString l_skipWord = l_bnd->getAttrValue("skipWord");
                        QString l_hideFilter = l_bnd->getAttrValue ("hideFilter");

                        l_hide = (l_hide.isEmpty () || l_hide.isNull ()) ? "no" : l_hide;
                        l_hideNext = (l_hideNext.isEmpty () || l_hideNext.isNull ()) ? "no" : l_hideNext;
                        l_skipWord = (l_skipWord.isEmpty () || l_skipWord.isNull ()) ? "yes" : l_skipWord;

                        // Attribute 'hide': Prevents this node (source node) from appearing on the next round of parsing. (default = 'no')
                        if ( !l_hideThis && !l_hideOther && l_hide == "no" )
                            l_ndLst.push_back ( const_cast<Node*>( dynamic_cast<const Node*> ( l_lnk->source () ) ) );
                        else {
                            //qDebug() << "(ling) [Meaning] *** Hid '" << l_lnk->source () << "' from appearing on the next pass of parsing.";
                        }

                        // Attribute 'hideNext': Prevents the next node (destination node) from appearing on its next round of parsing (2 rounds from now) (default = 'no')
                        if ( l_hideNext == "yes" ) {
                            l_hideOther = true;
                            //qDebug() << "(ling) [Meaning] *** Hid '" << l_lnk->destination () << "' from appearing on the next pass of parsing (2 rounds from now).";
                        } else l_hideOther = false;

                        // Attribute 'skipWord': Doesn't allow the destination node to have a chance at being parsed. (default = yes)
                        if ( l_skipWord == "yes" )
                            l_ndItr++;
                        else {
                            //qDebug() << "(ling) [Meaning] *** Skipping prevented for word-symbol '" << l_lnk->destination () << "'; will be parsed on next round.";
                        }


                        // Attribute 'hideFilter': Hides a set of words from appearing on the next round of parsing; a wrapper for the 'hide' attribute. (default = "")
                        if ( l_hideFilter.length () != 0 ) {
                            QStringList *l_e = new QStringList;

                            if ( l_hideFilter.contains ( "," ) ) {
                                QStringList d = l_hideFilter.split ( "," );
                                foreach ( const QString q, d )
                                    l_e->append ( q );
                            } else l_e->append ( l_hideFilter );

                            l_hideList = l_e;
                            //qDebug() << "(ling) [Meaning] *** Hiding any nodes that falls into the regex" << l_hideList->join (" 'or' ") << "on the next round.";
                        }
        #if 0
                        qDebug() << "(ling) [Meaning] Flags> hide: (" << l_hide
                                 << ") hideThis: ("   << ((l_hideThis == true) ? "yes" : "no")
                                 << ") hideOther: ("  << ((l_hideOther == true) ? "yes" : "no")
                                 << ") hideNext: ("   << l_hideNext
                                 << ") hideFilter: (" << l_hideFilter
                                 << ") skipWord: ("   << l_skipWord
                                 << ") hideList: ("   << ((l_hideList == NULL) ? "NULL" : "*") << ")"
                                 << endl << "Link sig: " << l_lnk->toString ().c_str () << endl;
        #endif

                    } else {
                        //qWarning() << "(ling) [Meaning] Linking failed ... horribly." << endl;
                        l_lnk = NULL;
                    }

                    //qDebug() << endl << "(ling) [Meaning] Nodes to be queued:" << endl << l_ndVtr << endl;

                }
                qDebug() << "(ling) [Meaning] Formed" << p_lnkLst->size () << "links with" << l_ndLst.size () << "nodes left to parse.";
            }

            if ( !p_lnkLst->empty () ) {
                if ( ! ( p_lnkLst->size () >= 1) || l_ndLst.size () > 0 ){
                    Q_ASSERT(Meaning::s_cnt < 5);
                    return Meaning::form ( &*p_lnkLst, l_ndLst );
                }
                else {
                    Meaning::s_cnt = 0;
                    return new Meaning ( *p_lnkLst );
                }
            } else
                return NULL;
        }

        const Link* Meaning::base () const {
            return m_lnkVtr.back ();
        }

        const LinkList* Meaning::siblings () const {
            return &m_lnkVtr;
        }

        const LinkList Meaning::linksAt(const int& p_lvl){
            LinkList l_lnkVtr;

            if (p_lvl > 1 && p_lvl <= levels()){
                foreach (Link* l_lnk, m_lnkVtr){
                    if (l_lnk->level() == p_lvl)
                        l_lnkVtr << l_lnk;
                    else continue;
                }
            } else
                qDebug() << "(ling) [Meaning] Out of level range.";

            return l_lnkVtr;
        }

        const int Meaning::levels() const {
            int l_lvl = 0;

            foreach (const Link* l_lnk, m_lnkVtr){
                if (l_lvl < l_lnk->level())
                    l_lvl = l_lnk->level();
                else continue;
            }

            return l_lvl;
        }

        const LinkList Meaning::isLinkedTo(const Node& p_nd) const {
            LinkList l_lnkVtr;

            foreach (Link* l_lnk, m_lnkVtr) {
                if (l_lnk->source () == &p_nd)
                    l_lnkVtr << l_lnk;
            }

            return l_lnkVtr;
        }

        const LinkList Meaning::isLinkedBy(const Node& p_nd) const {
            LinkList l_lnkVtr;

            foreach (Link* l_lnk, m_lnkVtr) {
                if (l_lnk->destination () == &p_nd)
                    l_lnkVtr << l_lnk;
            }

            return l_lnkVtr;
        }

        const QString Meaning::toText () const {
            if (levels() >= 2){
                qDebug() << "(ling) [Meaning] Has" << m_lnkVtr.size () << "link(s).";
                foreach (const Link* l_lnk, m_lnkVtr)
                    qDebug() << "(ling) [Meaning]" << l_lnk;
            } else {
                qDebug() << "(ling) [Meaning] Invalid meaning.";
            }

            return QString::null;
        }

        Meaning::~Meaning () { }
    }
}

// kate: indent-mode cstyle; space-indent on; indent-width 4;
