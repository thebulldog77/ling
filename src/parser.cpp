/**
 * @file    parser.cpp
 * @author  Wintermute Developers <wintermute-devel@lists.launchpad.net>
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * @endlegalese
 *
 * @todo Remove the connection to QtXml in this code and implement the read/write capabilities of RuleSets in WntrData::Linguistics.
 * @todo Add signals (binded and bindFailed).
 */

#include "syntax.hpp"
#include "parser.hpp"
#include "meanings.hpp"
#include <wntrdata.hpp>
#include <iostream>
#include <iomanip>
#include <QFile>
#include <QString>
#include <QVector>
#include <QtDebug>
#include <QTextStream>
#include <boost/tokenizer.hpp>
#include "syntax.hpp"

using namespace std;
using namespace Wintermute::Data::Linguistics;

using std::cout;
using std::endl;

namespace Wintermute {
    namespace Linguistics {
        Binding::Binding ( const Rules::Bond& p_bnd , const Rule* p_rl ) : m_bnd(p_bnd), m_rl(p_rl) { }

        const Binding* Binding::obtain ( const Node& p_nd, const Node& p_nd2 ) {
            const Rule* l_rl = Rule::obtain ( p_nd );
            if ( !l_rl ) return NULL;
            return l_rl->getBindingFor ( p_nd,p_nd2 );
        }

        const QString Binding::getAttrValue ( const QString &p_attr ) const {
            return m_bnd.attribute (p_attr);
        }

        /// @todo This method needs to match with more precision.
        const double Binding::canBind ( const Node &p_ndSrc, const Node& p_ndDst ) const {
            if ( this->parentRule ()->appliesFor ( p_ndSrc ) == 0.0 )
                return 0.0;

            const QString l_wh = m_bnd.attribute ( "with" );
            const QString l_ndDestStr = QString::fromStdString (p_ndDst.toString ( Node::EXTRA ));
            const QString l_ndSrcStr = QString::fromStdString (p_ndSrc.toString ( Node::EXTRA ));
            QStringList l_options = l_wh.split ( "," );

            foreach (const QString l_s, l_options) {
                double l_rtn = Rules::Bond::matches(l_ndDestStr,l_wh);

                if (l_rtn > 0.0) {
                    if (m_bnd.hasAttribute ("typeHas")){
                        const QString l_bindType = this->getAttrValue ( "typeHas" );
                        const double l_min = ((double) l_bindType.length () / 100.0);
                        const double l_matchVal = Rules::Bond::matches (l_ndSrcStr,l_bindType);

                        if ( l_min > l_matchVal ){
                            l_rtn = 0.0;
                            qDebug() << "(ling) [Binding] Required type:" << l_bindType << "in" << l_ndSrcStr << endl;
                        }
                    }

                    qDebug() << "(ling) [Binding] Bond:"<< l_rtn * 100 << "% for" << p_ndSrc.symbol () << "to" << p_ndDst.symbol () << "via" << l_wh;
                    return l_rtn;
                }
            }

            //qDebug() << "(ling) [Binding] Binding failed for (src) -> (dst) :" << p_ndSrc.toString (Node::EXTRA).c_str () << " -> " << l_ndDestStr.toStdString ().c_str () << " via" << l_wh;

            return 0.0;
        }

        /// @note This is where the attribute 'linkAction' is defined.
        const Link* Binding::bind ( const Node& p_nd1, const Node& p_nd2 ) const {
            if (!canBind(p_nd1,p_nd2)){
                emit bindFailed(const_cast<Binding*>(this),&p_nd1,&p_nd2);
                return NULL;
            }

            QString l_type = QString::fromStdString (this->parentRule ()->type ());
            QString l_lcl = QString::fromStdString (this->parentRule ()->locale());
            Node *l_nd = const_cast<Node*> ( &p_nd1 ), *l_nd2 = const_cast<Node*> ( &p_nd2 );

            if (m_bnd.hasAttribute ("linkAction")){
                const QStringList l_options = m_bnd.attribute ("linkAction").split (",");

                if (l_options.contains ("reverse")) {
                    l_type = QString::fromStdString (p_nd2.toString ( Node::MINIMAL )).at (0);
                    l_lcl = p_nd2.locale ();
                    Node *l_tmp = l_nd;
                    l_nd = l_nd2;
                    l_nd2 = l_tmp;
                } else if (l_options.contains ("othertype")){
                    l_type = p_nd2.toString ( Node::MINIMAL ).at (0);
                } else if (l_options.contains ("thistype")){
                    l_type = p_nd1.toString ( Node::MINIMAL ).at (0);
                }
            }

            emit binded(this,&p_nd1,&p_nd2);
            qDebug() << "(ling) [Binding] Link formed: " << p_nd1.toString (Node::EXTRA).c_str () << " " << p_nd2.toString (Node::EXTRA).c_str ();
            return Link::form ( *&l_nd, *&l_nd2 , l_type.toStdString () , l_lcl.toStdString () );
        }

        const Rule* Binding::parentRule () const {
            return m_rl;
        }

        Rule::Rule(const Rule& p_rl) : m_chn(p_rl.m_chn) { __init(); }

        Rule::Rule(const Rules::Chain &p_chn) : m_chn(p_chn) { __init(); }

        void Rule::__init() {
            foreach (Rules::Bond* l_bnd, m_chn.bonds())
                m_bndVtr.push_back ((new Binding(*&*l_bnd,this)));
        }

        const Rule* Rule::obtain ( const Node& p_nd ) {
            const QString l_lcl = p_nd.locale ();
            const QString l_flg = p_nd.flags ().begin ().value ();
            Rules::Chain l_chn(l_lcl,l_flg);
            Rules::Cache::read (l_chn);
            return new Rule(l_chn);
        }

        const Link* Rule::bind ( const Node& p_curNode, const Node& p_nextNode ) const {
            for ( BindingVector::const_iterator i = m_bndVtr.begin (); i != m_bndVtr.end (); i++ ) {
                const Binding* l_bnd = *i;
                if ( l_bnd->canBind ( p_curNode,p_nextNode ) )
                    return l_bnd->bind ( p_curNode,p_nextNode );
            }

            return NULL;
        }

        const bool Rule::canBind ( const Node& p_nd, const Node &p_dstNd ) const {
            for ( BindingVector::const_iterator i = m_bndVtr.begin (); i != m_bndVtr.end (); i++ ) {
                const Binding* l_bnd = *i;
                if ( l_bnd->canBind ( p_nd,p_dstNd ) )
                    return true;
            }

            return false;
        }

        const Binding* Rule::getBindingFor ( const Node& p_nd, const Node& p_nd2 ) const {
            map<const double, const Binding*> l_bndLevel;
            for ( BindingVector::const_iterator i = m_bndVtr.begin (); i != m_bndVtr.end (); i++ ) {
                const Binding* l_bnd = *i;
                const double l_vl = l_bnd->canBind ( p_nd,p_nd2 );
                if ( l_vl ){
                    l_bndLevel.insert (map<const double,const Binding*>::value_type(l_vl,l_bnd));
                    qDebug() << "(ling) [Rule] Valid binding for" << p_nd.symbol () << "to" << p_nd2.symbol ();
                }
            }

            if (!l_bndLevel.empty ()){
                map<const double,const Binding*>::const_iterator l_itr = l_bndLevel.begin ();
                return l_itr->second;
            } else return NULL;
        }

        /// @todo This method needs to match with more precision.
        const double Rule::appliesFor ( const Node& p_nd ) const {
            const QString l_ndStr ( p_nd.toString ( Node::EXTRA ).c_str () );
            const QString l_rlStr ( type().c_str () );
            const double l_rtn = Rules::Bond::matches(l_ndStr,l_rlStr);

            return l_rtn;
        }

        const string Rule::type() const { return m_chn.type ().toStdString (); }

        const string Rule::locale () const { return m_chn.locale().toStdString(); }

        Parser::Parser ( const string& p_lcl ) : m_lcl ( p_lcl ) { }

        const string Parser::locale () const {
            return m_lcl;
        }

        void Parser::setLocale ( const string& p_lcl ) {
            m_lcl = p_lcl;
        }

        /// @todo Need to find a way to add more information about the symbol parsed here. Did it have a period, comma, or even a semi-colon?
        QStringList Parser::getTokens ( string const &p_str ) {
            boost::tokenizer<> l_toks(p_str);
            boost::tokenizer<>::iterator itr = l_toks.begin ();
            QStringList l_strLst;
            for ( ; itr != l_toks.end (); itr++)
                l_strLst << QString::fromStdString (*itr);

            return l_strLst;
        }

        void Parser::generateNode(Node* p_nd){
            cout << "(ling) [Parser] Encountered unrecognizable word (" << p_nd->symbol ().toStdString () << "). " << endl
                 << setw(5) << right << setfill(' ')
                 << "Add to system? ( yes / [n]o): ";

            QTextStream l_iStrm(stdin);

            if (l_iStrm.readLine () == "yes"){
                const Lexical::Data* l_dt = p_nd->data();
                cout << "(ling) Enter lexical flags in such a manner; ONTOID LEXIDATA. Press <ENTER> twice to complete the flag entering process." << endl;
                QString l_oid, l_flg, l_ln = l_iStrm.readLine ();
                Lexical::DataFlagMap l_dtmp;

                while (!l_ln.isNull() && !l_ln.isEmpty ()) {
                    QStringList l_objs = l_ln.split (" ");
                    l_oid = l_objs[0];
                    l_flg = l_objs[1];

                    l_dtmp.insert(l_oid,l_flg);
                    l_ln = l_iStrm.readLine ();
                }

				Lexical::Data l_nwDt = Lexical::Data::createData(Lexical::Data::idFromString(p_nd->symbol()),
								 QString::fromStdString(locale()), p_nd->symbol(), l_dtmp);
				Lexical::Cache::write(l_nwDt);
				p_nd = new Node(l_nwDt);
				qDebug() << "(ling) [Parser] Node generated." << endl;
			} else {
				qDebug() << "(ling) [Parser] Node creation cancelled." << endl;
				p_nd = NULL;
			}
		}

        /// @todo Allow a handle to be created here whenever it bumps into a foreign word.
        NodeList Parser::formNodes ( QStringList const &p_tokens ) {
            NodeList l_theNodes;
            connect(this,SIGNAL(foundPseduoNode(Node*)), this,SLOT(generateNode(Node*)));

            foreach(QString l_token, p_tokens)
                l_theNodes.push_back(formNode(l_token));

            disconnect(this,SLOT(generateNode(Node*)));
            return l_theNodes;
        }

        /// @todo Add more information to how the Node is presented (like punctaction).
        /// @note An assumption is made here (that the QRegExp splits it into three parts). If someone were to enter "libro?!?"
        Node* Parser::formNode( QString const &p_symbol ){
            const string l_theID = Lexical::Data::idFromString (p_symbol).toStdString();
            Node* l_theNode = const_cast<Node*>(Node::obtain (m_lcl,l_theID));

            if ( !Node::exists (m_lcl,l_theID) ) {
                string l_sym(p_symbol.toStdString ());
                l_theNode = const_cast<Node*>(Node::buildPseudo ( m_lcl , l_sym ));
                emit foundPseduoNode(l_theNode);
            }

            l_theNode->setProperty ("OriginalToken",p_symbol);
            return l_theNode;
        }

        /// @todo Find a means of reporting progress from this method; this method can end up becoming extremely time-consuming.
        NodeTree Parser::expandNodes ( NodeTree& p_tree, const int& p_size, const int& p_level ) {
            // Salvaged this method's algorithm from an older version of the parser.

            if ( p_level == p_tree.size () ){
                qDebug() << "(ling) Flag unwinding complete.";
                return ( NodeTree() );
            }

            const NodeList l_curBranch = p_tree.at ( p_level );
            const bool isAtEnd = ( p_level + 1 == p_tree.size () );

            if ( l_curBranch.empty () ) {
                qDebug() << "(ling) [Parser] WARNING: Null data detected at level" << p_level << ".";
                return ( NodeTree() );
            }

            const int l_mxSize = p_size / l_curBranch.size ( );

            NodeTree l_chldBranches, l_foundStems = expandNodes ( p_tree , l_mxSize , p_level + 1 );

            for ( NodeList::ConstIterator jtr = l_curBranch.begin ( ); jtr != l_curBranch.end ( ); jtr ++ ) {
                const Node* l_curLvlNd = * jtr;

                if ( !isAtEnd ) {
                    for ( NodeTree::iterator itr = l_foundStems.begin ( ); itr != l_foundStems.end ( ); itr ++ ) {
                        NodeList l_tmpVector; // creates the current vector (1 of x, x = l_curBranch.size();
                        l_tmpVector << (const_cast<Node*>(l_curLvlNd)) << (*itr);
                        l_chldBranches.push_back ( l_tmpVector ); // add this current branch to list.
                    }
                } else { // the end of the line!
                    NodeList tmpVector;
                    tmpVector << (const_cast<Node*>(l_curLvlNd));
                    l_chldBranches.push_back ( tmpVector ); // add this current branch to list.
                }
            }

            qDebug() << "(ling) [Parser] Tier" << (p_tree.size () - p_level) << ((l_chldBranches.size () != p_size) ? (QString("generated") + QString::number (l_chldBranches.size()) + QString("of its") + QString(p_size)) : (QString("all of its"))).toStdString ().c_str () << "expected branches.";
            return l_chldBranches;
        }

        NodeTree Parser::expandNodes ( NodeList const &p_ndVtr ) {
            int l_totalPaths = 1;
            NodeTree l_metaTree;

            for ( NodeList::ConstIterator itr = p_ndVtr.begin (); itr != p_ndVtr.end (); itr++ ) {
                const Node* l_nd = *itr;
                NodeList l_variations = Node::expand ( l_nd );
                const unsigned int size = l_variations.size ();
                Q_ASSERT(size >= 1);
                if ( itr != p_ndVtr.begin() )
                    l_totalPaths *= size;

                l_metaTree.push_back ( l_variations );
            }

            qDebug() << "(ling) [Parser] Expecting" << l_totalPaths << "path(s).";

            emit unwindingProgress(0.0);
            NodeTree l_tree = expandNodes ( l_metaTree , l_totalPaths , 0 );
            emit unwindingProgress(1.0);

            qDebug() << "(ling) [Parser] Found" << l_tree.size() << "path(s).";

            emit finishedUnwinding();
            return l_tree;
        }

        /// @todo Determine a means of generating unique signatures.
        const string Parser::formShorthand ( const NodeList& p_ndVtr, const Node::FormatVerbosity& p_sigVerb ) {
            string l_ndShrthnd;

            for ( NodeList::const_iterator itr = p_ndVtr.begin (); itr != p_ndVtr.end (); ++itr ) {
                const Node* l_nd = *itr;
                l_ndShrthnd += l_nd->toString ( p_sigVerb );
            }

            return l_ndShrthnd;
        }

        /// @todo When parsing multiple sentences back-to-back; we need to implement a means of maintaining context.
        void Parser::parse ( const string& p_txt ) {
            QTextStream l_strm(p_txt.c_str (),QIODevice::ReadOnly);
            MeaningVector l_mngVtr;

            while (!l_strm.atEnd ()){
                QString l_str = l_strm.readLine ();
                QStringList l_sentences = l_str.split (QRegExp("[.!?;]\\s"),QString::SkipEmptyParts);

                foreach (QString l_sentence, l_sentences){
                    if (l_sentences.front () != l_sentence)
                        qDebug() << "Parsing next sentence...";

                    Meaning* l_mng = const_cast<Meaning*>( process ( l_sentence.toStdString() ) );
                    if (l_mng) {
                        l_mngVtr.push_back (l_mng);
                    }
                }
            }
        }

        /// @todo Obtain the one meaning that represents the entire parsed text.
        const Meaning* Parser::process ( const string& p_txt ) {
            QStringList l_tokens = getTokens ( p_txt );
            NodeList l_theNodes = formNodes ( l_tokens );
            NodeTree l_nodeTree = expandNodes ( l_theNodes );

            MeaningVector l_meaningVtr;
            for ( NodeTree::const_iterator itr = l_nodeTree.begin (); itr != l_nodeTree.end (); itr++ ) {
                const NodeList l_ndVtr = *itr;
                qDebug() << "(ling) [Parser] Forming meaning #" << (l_meaningVtr.size () + 1) << "...";
                Meaning* l_meaning = const_cast<Meaning*>(Meaning::form ( NULL, l_ndVtr ));
                if (l_meaning != NULL)
                    l_meaningVtr.push_back ( l_meaning );
            }

            unique ( l_meaningVtr.begin(),l_meaningVtr.end () );
            qDebug() << "(ling) [Parser]" << l_nodeTree.size () << "paths formed" << l_meaningVtr.size () << "meanings.";
            cout << endl << setw(20) << setfill('=') << " " << endl;

            for ( MeaningVector::const_iterator itr2 = l_meaningVtr.begin (); itr2 != l_meaningVtr.end (); itr2++ ) {
                const Meaning* l_mngItr = *itr2;
                l_mngItr->toText ();
            }

            if (!l_meaningVtr.empty ()) return l_meaningVtr.front ();
            else return NULL;
        }


    }
}

// kate: indent-mode cstyle; space-indent on; indent-width 4;
