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
 * @todo Add signals (binded and bindFailed).
 */

#include "syntax.hpp"
#include "parser.hpp"
#include "meanings.hpp"
#include <wntrdata.hpp>
#include <iostream>
#include <iomanip>
#include <QFile>
#include <QtXml>
#include <QString>
#include <QVector>
#include <QtDebug>
#include <QTextStream>
#include <boost/tokenizer.hpp>
#include "syntax.hpp"

using namespace std;

using std::cout;
using std::endl;
using Wintermute::Data::Linguistics::Lexical::SaveModel;
using Wintermute::Data::Linguistics::Lexical::Storage;

namespace Wintermute {
    namespace Linguistics {
        RuleSetMap RuleSet::s_rsm;

        Binding::Binding ( QDomElement p_ele, const Rule* p_rl ) : m_rl ( p_rl ), m_ele ( p_ele ){ }

        const Binding* Binding::obtain ( const Node& p_nd, const Node& p_nd2 ) {
            const Rule* l_rl = Rule::obtain ( p_nd );
            if ( !l_rl ) return NULL;
            return l_rl->getBindingFor ( p_nd,p_nd2 );
        }

        const QString Binding::getAttrValue ( const QString &p_attr ) const {
            return m_ele.attribute ( p_attr );
        }

        /// @todo This method needs to match with more precision.
        const double Binding::canBind ( const Node &p_nd, const Node& p_nd2 ) const {
            if ( !this->parentRule ()->appliesFor ( p_nd ) )
                return 0.0;

            const QString l_wh = this->m_ele.attribute ( "with" );
            const QString l_ndStr ( p_nd2.toString ( Node::EXTRA ).c_str () );
            QStringList l_options = l_wh.split ( "," );

            for ( QStringList::ConstIterator l_itr = l_options.begin (); l_itr != l_options.end (); l_itr++ ) {
                const QString l_s = *l_itr;
                const bool l_half = (l_ndStr.contains ( l_s ) || l_ndStr.indexOf ( l_s ) != -1);
                const bool l_full = (l_ndStr == l_s);
                double l_rtn = 0.0;

                if (l_half && !l_full) l_rtn = 0.5;
                else if ((!l_half && l_full) || (l_half && l_full)) l_rtn = 1.0;

                if (m_ele.hasAttribute ("typeHas")){
                    qDebug() << "(ling) [Binding] Required type!" << endl;
                    const QString l_hasTypeHas ( p_nd.toString ( Node::EXTRA ).c_str () );
                    const QString l_bindType = this->getAttrValue ( "typeHas" );

                    if ( !l_bindType.isEmpty () ) {
                        if ( ! l_bindType.contains ( l_hasTypeHas ) )
                            l_rtn = 0.0;
                    }
                }

                if (l_rtn != 0.0) {
                    qDebug() << "(ling) [Binding] Bond: "<< l_rtn * 100 << "% for '" << p_nd.symbol () << "' to '" << p_nd2.symbol () << "'";
                    return l_rtn;
                }
            }

            qDebug() << "(ling) [Binding] Binding failed for %ND2 -> %ND1 : " << p_nd.toString (Node::EXTRA).c_str ()<< " " << l_ndStr;

            return 0.0;
        }

        /// @note This is where the attribute 'linkAction' is defined.
        const Link* Binding::bind ( const Node& p_nd1, const Node& p_nd2 ) const {
            if (!canBind(p_nd1,p_nd2)){
                emit bindFailed(const_cast<Binding*>(this),&p_nd1,&p_nd2);
                return NULL;
            }

            QString l_type = this->parentRule ()->type ().c_str ();
            QString l_lcl = this->parentRule ()->parentRuleSet ()->locale ().c_str ();
            Node *l_nd = const_cast<Node*> ( &p_nd1 ), *l_nd2 = const_cast<Node*> ( &p_nd2 );

            if (m_ele.hasAttribute ("linkAction")){
                const QStringList l_eleStr = m_ele.attribute ("linkAction").split (",");

                if (l_eleStr.contains ("reverse")) {
                    l_type = p_nd2.toString ( Node::MINIMAL ).c_str ();
                    l_lcl = p_nd2.locale ().toStdString ().c_str ();
                    Node *l_tmp = l_nd;
                    l_nd = l_nd2;
                    l_nd2 = l_tmp;
                    //qDebug() << "(ling) [Binding] Reversed the type and locale of the link.";
                } else if (l_eleStr.contains ("othertype")){
                    l_type = p_nd2.toString ( Node::MINIMAL ).c_str ();
                } else if (l_eleStr.contains ("thistype")){
                    l_type = p_nd1.toString ( Node::MINIMAL ).c_str ();
                }
            }

            emit binded(this,&p_nd1,&p_nd2);
            return Link::form ( *&l_nd, *&l_nd2 , l_type.toStdString () , l_lcl.toStdString () );
        }

        const Rule* Binding::parentRule () const {
            return m_rl;
        }

        void Rule::__init() {
            QDomNodeList l_ndlst = m_ele.elementsByTagName ( "Bind" );

            for ( int i = 0; i < l_ndlst.count (); i++ ) {
                QDomElement l_e = l_ndlst.at ( i ).toElement ();
                m_bndVtr.push_back ( ( new Binding ( l_e,this ) ) );
            }
        }

        const Rule* Rule::obtain ( const Node& p_nd ) {
            const RuleSet* l_rlst = RuleSet::obtain ( p_nd );
            return l_rlst->getRuleFor ( p_nd );
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
                if ( l_vl > 0.0 || l_vl == 1.0 ){
                    l_bndLevel.insert (map<const double,const Binding*>::value_type(l_vl,l_bnd));
                    qDebug() << "(ling) [Rule] Valid binding for '" << p_nd.symbol () << "' to '" << p_nd2.symbol () << "'";
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
            const bool l_half = ( l_ndStr.contains ( l_rlStr ) || l_ndStr.indexOf ( l_rlStr ) != -1 );
            const bool l_full = l_ndStr == l_rlStr;
            double l_rtn = 0.0;

            if ((!l_half && l_full) || (l_half && l_full)) l_rtn = 1.0;
            else if (l_half && !l_full) l_rtn = 0.5;

            if (l_rtn != 0.0) {
                qDebug() << "(ling) [Rule] Bond: " << l_rtn * 100 << "% for '" << p_nd.symbol () << "'";
            }

            return l_rtn;
        }

        const string Rule::type() const {
            return m_ele.attribute ( "type" ).toStdString ();
        }

        const RuleSet* Rule::parentRuleSet () const {
            return m_rlst;
        }

        /// @todo Remove the checks for m_dom and m_rules and just have them clear the previous information in each value upon load.
        void RuleSet::__init ( const string& p_lcl ) {
            const string l_pth = Data::Linguistics::Configuration::directory () + string ( "/" ) + p_lcl + string ( "/grammar.xml" );
            qDebug() << "(ling) [RuleSet] Loading '" << l_pth.c_str () << "'...";
            QFile *l_rlstDoc = new QFile ( QString ( l_pth.c_str () ) );

            if (!l_rlstDoc->open ( QIODevice::ReadOnly )){
                qWarning() << "(ling) [RuleSet] Failed to read data from RuleSet document!";
                return;
            }

            if (!m_dom) m_dom = new QDomDocument;

            {
                QString l_errorMsg;
                int l_errorLine, l_errorColumn;
                if (!m_dom->setContent ( l_rlstDoc , &l_errorMsg, &l_errorLine, &l_errorColumn )){
                    qDebug () << "(ling) [RuleSet] Failed to load XML data for RuleSet. Error:" << l_errorMsg << "on line" << l_errorLine << "at column" << l_errorColumn;
                    return;
                }
            }

            QDomElement l_ele = m_dom->documentElement ();
            QDomNodeList l_ruleNdLst = l_ele.elementsByTagName ( "Rule" );

            if ( !m_rules->empty() ) m_rules->clear();

            for ( int i = 0; i < l_ruleNdLst.count (); i++ ) {
                QDomNode l_domNd = l_ruleNdLst.at ( i );
                QDomElement l_curEle = l_domNd.toElement ();
                if ( l_curEle.tagName () == "Rule" )
                    m_rules->push_back ( new Rule ( l_curEle,this ) );
            }

            qDebug() << "(ling) [RuleSet] Loaded " << m_rules->size () << " of " << l_ruleNdLst.count () << " rules." << endl;
        }

        RuleSet* RuleSet::obtain ( const Node& p_ndRef ) {
            RuleSet* l_rlst = NULL;

            if ( s_rsm.find ( p_ndRef.locale ().toStdString ()) == s_rsm.end() ) {
                l_rlst = new RuleSet ( p_ndRef.locale ().toStdString ());
                s_rsm.insert ( RuleSetMap::value_type ( p_ndRef.locale ().toStdString () ,l_rlst ) );
            } else
                l_rlst = s_rsm.find ( p_ndRef.locale ().toStdString () )->second;

            return l_rlst;
        }

        /// @todo This method needs to let other rules get a chance.
        const Rule* RuleSet::getRuleFor ( const Node& p_nd ) const {
            map<const double, const Rule*> l_rlVtr;
            for ( RuleVector::const_iterator i = m_rules->begin (); i != m_rules->end (); i++ ) {
                const Rule* l_rl = *i;
                const double l_rt = l_rl->appliesFor (p_nd);
                if ( l_rt > 0.0 || l_rt == 1.0 ){
                    l_rlVtr.insert (map<const double,const Rule*>::value_type(l_rt,l_rl));
                    qDebug() << "(ling) [RuleSet] Valid rule for '" << p_nd.symbol () << "'";
                }
            }

            if (!l_rlVtr.empty ()){
                map<const double,const Rule*>::const_iterator l_itr = l_rlVtr.begin ();
                return (l_itr->second);
            } else return NULL;
        }

        void RuleSet::setLocale(const string& p_lcl){
            __init(p_lcl);
        }

        const Binding* RuleSet::getBindingFor ( const Node& p_nd, const Node& p_nd2 ) const {
            const Rule* l_rl = getRuleFor ( p_nd );
            if ( l_rl )
                return l_rl->getBindingFor ( p_nd,p_nd2 );
            else
                return NULL;
        }

        const bool RuleSet::canBind ( const Node& p_nd, const Node& p_nd2 ) const {
            return ( this->getBindingFor ( p_nd,p_nd2 ) != NULL );
        }

        const Link* RuleSet::bind ( const Node& p_nd, const Node& p_nd2 ) const {
            const Binding* l_bnd = this->getBindingFor ( p_nd,p_nd2 );
            if ( l_bnd )
                return l_bnd->bind ( p_nd,p_nd2 );
            else
                return NULL;
        }

        const string RuleSet::locale() const {
            return m_dom->documentElement ().attribute ( "locale" ).toStdString();
        }

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
                cout << "(ling) Enter lexical flags in such a manner; ONTOID LEXIDATA. Press <ENTER> twice to quit." << endl;
                QString l_oid, l_flg, l_ln = l_iStrm.readLine ();
                Lexical::DataFlagMap l_dtmp;

                while (!l_ln.isNull() && !l_ln.isEmpty ()) {
                    qDebug() << l_ln;
                    QStringList l_objs = l_ln.split (" ");
                    l_oid = l_objs[0];
                    l_flg = l_objs[1];

                    l_dtmp.insert(&l_oid,&l_flg);
                    l_ln = l_iStrm.readLine ();
                }

                const Lexical::Data l_nwDt = Lexical::Data::createData (l_dt->id (),l_dt->locale (), l_dt->symbol (), l_dtmp);
                SaveModel* l_svmdl = Storage::obtain (&l_nwDt);
                l_svmdl->save ();
                p_nd = new Node(*l_svmdl->data ());
                qDebug() << "(ling) [Parser] Node generated.";
            } else {
                qDebug() << "(ling) [Parser] Node creation cancelled.";
                p_nd = NULL;
            }
        }

        /// @todo Allow a handle to be created here whenever it bumps into a foreign word.
        NodeVector Parser::formNodes ( QStringList const &p_tokens ) {
            NodeVector l_theNodes;
            connect(this,SIGNAL(foundPseduoNode(Node*)), this,SLOT(generateNode(Node*)));

            foreach(QString l_token, p_tokens)
                l_theNodes.push_back(formNode(l_token));

            disconnect(this,SLOT(generateNode(Node*)));
            return l_theNodes;
        }

        /// @todo Add more information to how the Node is presented (like punctaction).
        /// @note An assumption is made here (that the QRegExp splits it into three parts). If someone were to enter "libro?!?"
        Node* Parser::formNode( QString const &p_symbol ){
            const string l_theID = md5(p_symbol.toLower ().toStdString ());
            Node* l_theNode = const_cast<Node*>(Node::obtain (m_lcl,l_theID));

            if ( !Node::exists (m_lcl,l_theID) ) {
                string l_sym(p_symbol.toStdString ());
                l_theNode = const_cast<Node*>(Node::buildPseudo ( l_theID, m_lcl , l_sym ));
                emit foundPseduoNode(l_theNode);
            }

            l_theNode->setProperty ("OriginalToken",p_symbol);
            return l_theNode;
        }

        /// @todo Find a means of reporting progress from this method; this method can end up becoming extremely time-consuming.
        NodeTree Parser::expandNodes ( NodeTree& p_tree, const int& p_size, const int& p_level ) {
            // Salvaged this method's algorithm from an older version of the parser.

            if ( p_level == p_tree.size () )
                return ( NodeTree() );

            qDebug() << "(ling) [Parser] Level " << p_level << " should generate " << p_size << " paths.";
            const NodeVector l_curBranch = p_tree.at ( p_level );
            const bool isAtEnd = ( p_level + 1 == p_tree.size () );

            if ( l_curBranch.empty () ) {
                qDebug() << "(ling) [Parser] WARNING! Invalid level detected at level " << p_level << ".";
                return ( NodeTree() );
            }

            qDebug() << "(ling) [Parser] Level " << p_level << " has " << l_curBranch.size() << " variations.";

            const int l_mxSize = p_size / l_curBranch.size ( );

            NodeTree l_chldBranches, l_foundStems = expandNodes ( p_tree , l_mxSize , p_level + 1 );

            qDebug() << "(ling) [Parser] Level " << p_level << " expects " << l_foundStems.size() * l_curBranch.size () << " paths.";
            for ( NodeVector::ConstIterator jtr = l_curBranch.begin ( ); jtr != l_curBranch.end ( ); jtr ++ ) {
                const Node* l_curLvlNd = * jtr;

                if ( !isAtEnd ) {
                    for ( NodeTree::iterator itr = l_foundStems.begin ( ); itr != l_foundStems.end ( ); itr ++ ) {
                        NodeVector l_tmpVector; // creates the current vector (1 of x, x = l_curBranch.size();
                        l_tmpVector << (const_cast<Node*>(l_curLvlNd)) << (*itr);
                        l_chldBranches.push_back ( l_tmpVector ); // add this current branch to list.
                    }
                } else { // the end of the line!
                    NodeVector tmpVector;
                    tmpVector << (const_cast<Node*>(l_curLvlNd));
                    l_chldBranches.push_back ( tmpVector ); // add this current branch to list.
                }
            }

            qDebug() << "(ling) [Parser] Level" << p_level << "generated" << l_chldBranches.size() << "branches.";
            return l_chldBranches;
        }

        NodeTree Parser::expandNodes ( NodeVector const &p_ndVtr ) {
            int l_totalPaths = 1;
            NodeTree l_metaTree;

            for ( NodeVector::ConstIterator itr = p_ndVtr.begin (); itr != p_ndVtr.end (); itr++ ) {
                const Node* l_nd = *itr;
                NodeVector l_variations = Node::expand ( l_nd );
                const unsigned int size = l_variations.size ();

                if ( itr != p_ndVtr.begin() )
                    l_totalPaths *= size;

                l_metaTree.push_back ( l_variations );
            }

            qDebug() << "(ling) [Parser] Expanding across" << p_ndVtr.size () << "levels and expecting" << l_totalPaths << "different paths...";
            emit unwindingProgress(0.0);
            NodeTree l_tree = expandNodes ( l_metaTree , l_totalPaths , 0 );
            emit unwindingProgress(1.0);

            qDebug() << "(ling) [Parser] Found" << l_tree.size() << "path(s).";

            emit finishedUnwinding();
            return l_tree;
        }

        /// @todo Determine a means of generating unique signatures.
        const string Parser::formShorthand ( const NodeVector& p_ndVtr, const Node::FormatVerbosity& p_sigVerb ) {
            string l_ndShrthnd;

            for ( NodeVector::const_iterator itr = p_ndVtr.begin (); itr != p_ndVtr.end (); ++itr ) {
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
            NodeVector l_theNodes = formNodes ( l_tokens );
            NodeTree l_nodeTree = expandNodes ( l_theNodes );

            MeaningVector l_meaningVtr;
            for ( NodeTree::const_iterator itr = l_nodeTree.begin (); itr != l_nodeTree.end (); itr++ ) {
                const NodeVector l_ndVtr = *itr;
                qDebug() << "(ling) [Parser] " << "Forming meaning #" << (l_meaningVtr.size () + 1) << "...";
                Meaning* l_meaning = const_cast<Meaning*>(Meaning::form ( NULL, l_ndVtr ));
                if (l_meaning != NULL)
                    l_meaningVtr.push_back ( l_meaning );
            }

            unique ( l_meaningVtr.begin(),l_meaningVtr.end () );
            qDebug() << "(ling) [Parser]" << l_nodeTree.size () << "paths formed" << l_meaningVtr.size () << "meanings.";
            cout << endl << setw(20) << setfill('=') << " " << endl;

            for ( MeaningVector::const_iterator itr2 = l_meaningVtr.begin (); itr2 != l_meaningVtr.end (); itr2++ ) {
                const Meaning* l_mngItr = *itr2;
                qDebug() << l_mngItr->toText () << endl;
            }

            if (!l_meaningVtr.empty ()) return l_meaningVtr.front ();
            else return NULL;
        }


    }
}

// kate: indent-mode cstyle; space-indent on; indent-width 4;
