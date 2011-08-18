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
#include <QtDebug>
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
                    //cout << "(ling) [Binding] Required type!" << endl;
                    const QString l_hasTypeHas ( p_nd.toString ( Node::EXTRA ).c_str () );
                    const QString l_bindType = this->getAttrValue ( "typeHas" );

                    if ( !l_bindType.isEmpty () ) {
                        if ( ! l_bindType.contains ( l_hasTypeHas ) )
                            l_rtn = 0.0;
                    }
                }

                if (l_rtn != 0.0) {
                    //cout << "(ling) [Binding] Bond: "<< l_rtn * 100 << "% for '" << p_nd.symbol () << "' to '" << p_nd2.symbol () << "'" << endl;
                    return l_rtn;
                }
            }

            //cout << "(ling) [Binding] Binding failed for %ND2 -> %ND1 : " << p_nd.toString (Node::EXTRA) << " " << qPrintable(l_ndStr) << endl;

            return 0.0;
        }

        /// @note This is where the attribute 'linkAction' is defined.
        const Link* Binding::bind ( const Node& p_nd1, const Node& p_nd2 ) const {
            if (!canBind(p_nd1,p_nd2)) return NULL;

            string l_type = this->parentRule ()->type ();
            string l_lcl = this->parentRule ()->parentRuleSet ()->locale ();
            Node *l_nd = const_cast<Node*> ( &p_nd1 ), *l_nd2 = const_cast<Node*> ( &p_nd2 );

            if (m_ele.hasAttribute ("linkAction")){
                const QStringList l_eleStr = m_ele.attribute ("linkAction").split (",");

                if (l_eleStr.contains ("reverse")) {
                    l_type = p_nd2.toString ( Node::MINIMAL );
                    l_lcl = p_nd2.locale ();
                    Node *l_tmp = l_nd;
                    l_nd = l_nd2;
                    l_nd2 = l_tmp;
                    //cout << "(ling) [Binding] Reversed the type and locale of the link." << endl;
                } else if (l_eleStr.contains ("othertype")){
                    l_type = p_nd2.toString ( Node::MINIMAL );
                } else if (l_eleStr.contains ("thistype")){
                    l_type = p_nd1.toString ( Node::MINIMAL );
                }
            }

            return Link::form ( static_cast<const FlatNode*> ( l_nd ), static_cast<const FlatNode*> ( l_nd2 ), l_type, l_lcl );
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
                    //cout << "(ling) [Rule] Valid binding for '" << p_nd.symbol () << "' to '" << p_nd2.symbol () << "'" << endl;
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
                //cout << "(ling) [Rule] Bond: " << l_rtn * 100 << "% for '" << p_nd.symbol () << "'" << endl;
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
            const string l_dir = Data::Linguistics::Configuration::directory () + string ( "/locale/" ) + p_lcl + string ( "/rules.dat" );
            //cout << "(ling) [RuleSet] Loading '" << l_dir << "'..." << endl;
            QFile *l_rlstDoc = new QFile ( QString ( l_dir.c_str () ) );

            if (!l_rlstDoc->open ( QIODevice::ReadOnly )){
                //cout << "(ling) [RuleSet] Failed to read data from RuleSet document!" << endl;
                return;
            }

            if (!m_dom) m_dom = new QDomDocument;

            {
                QString l_errorMsg;
                int l_errorLine, l_errorColumn;
                if (!m_dom->setContent ( l_rlstDoc , &l_errorMsg, &l_errorLine, &l_errorColumn )){
                    //qDebug () << "(ling) [RuleSet] Failed to load XML data for RuleSet. Error:" << l_errorMsg << "on line" << l_errorLine << "at column" << l_errorColumn;
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

            //cout << "(ling) [RuleSet] Loaded " << m_rules->size () << " of " << l_ruleNdLst.count () << " rules." << endl;
        }

        RuleSet* RuleSet::obtain ( const Node& p_ndRef ) {
            RuleSet* l_rlst = NULL;

            if ( s_rsm.find ( p_ndRef.locale () ) == s_rsm.end() ) {
                l_rlst = new RuleSet ( p_ndRef.locale () );
                s_rsm.insert ( RuleSetMap::value_type ( p_ndRef.locale (),l_rlst ) );
            } else
                l_rlst = s_rsm.find ( p_ndRef.locale () )->second;

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
                    //cout << "(ling) [RuleSet] Valid rule for '" << p_nd.symbol () << "'" << endl;
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

        StringVector Parser::getTokens ( const string& p_str ) {
            tokenizer<> tkn ( p_str );
            StringVector l_theTokens;

            for ( tokenizer<>::const_iterator itr = tkn.begin (); itr != tkn.end (); ++itr ) {
                const QString l_str ( ( *itr ).c_str() );
                l_theTokens.push_back ( l_str.toLower ().toStdString () );
            }

            return l_theTokens;
        }

        void Parser::generateNode(const Node* p_nd){
            cout << "(ling) [Parser] The parser has encountered an unrecognized word. " << endl
                 << setw(5) << right << setfill(' ')
                 << "Do you want to add this to the system? [y]/n : ";

            char l_rs;
            cin >> l_rs;

            if (l_rs == 'y'){
                Lexidata* l_lxdt = const_cast<Lexidata*>(p_nd->data());
                cout << "(ling) Enter lexical flags in such a manner; ONTOID LEXIDATA. Press <ENTER> twice to quit." << endl;
                QTextStream l_inStrm(stdin);
                QString l_oid, l_flg, l_ln = l_inStrm.readLine ();
                Leximap l_lxmp;

                do {
                    QStringList l_objs = l_ln.split (" ");
                    l_oid = l_objs[0];
                    l_flg = l_objs[1];

                    l_lxmp.insert(Leximap::value_type(l_oid.toStdString (),l_flg.toStdString ()));
                    l_ln = l_inStrm.readLine ();
                } while (!l_ln.isNull());

                l_lxdt = new Lexidata(l_lxdt->id (),l_lxdt->locale (), l_lxdt->symbol (), l_lxmp);

                SaveModel* l_svmdl = Storage::obtain (l_lxdt);
                l_svmdl->save ();
                p_nd = Node::obtain (*(l_lxdt->locale ()),*(l_lxdt->id ()));
                qDebug() << "(ling) [Parser] Node generated.";
            } else {
                qDebug() << "(ling) [Parser] Node creation cancelled.";
                p_nd = NULL;
            }
        }

        /// @todo Allow a handle to be created here whenever it bumps into a foreign word.
        NodeVector Parser::formNodes ( StringVector& l_tokens ) {
            NodeVector l_theNodes;
            connect(this,SIGNAL(foundPseduoNode(const Node*)), this,SLOT(generateNode(const Node*)));

            for ( StringVector::const_iterator itr = l_tokens.begin (); itr != l_tokens.end (); itr++ )
                l_theNodes.push_back(formNode(*itr));

            disconnect(this,SLOT(generateNode(const Node*)));
            return l_theNodes;
        }

        Node* Parser::formNode( const string& p_token ){
            const string l_theID = md5 ( p_token );
            Node* l_theNode = const_cast<Node*>(Node::obtain ( m_lcl, l_theID ));

            if ( Node::exists (m_lcl,l_theID) )
                return l_theNode;
            else {
                l_theNode = const_cast<Node*>(Node::buildPseudo ( m_lcl, l_theID, p_token ));
                emit foundPseduoNode(l_theNode);
                return l_theNode;
            }

        }

        /// @todo Find a means of reporting progress from this method; this method can end up becoming extremely time-consuming.
        NodeTree Parser::expandNodes ( NodeTree& p_tree, const int& p_size, const int& p_level ) {
            // Salvaged this method's algorithm from an older version of the parser.

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

            //qDebug() << "(ling) [Parser] Level" << p_level << "generated" << l_chldBranches.size() << "branches." << endl;
            return l_chldBranches;
        }

        NodeTree Parser::expandNodes ( NodeVector& p_ndVtr ) {
            int l_totalPaths = 1;
            NodeTree l_metaTree;

            for ( NodeVector::ConstIterator itr = p_ndVtr.begin (); itr != p_ndVtr.end (); itr++ ) {
                const Node* l_nd = *itr;
                NodeVector l_variations = FlatNode::expand ( l_nd );
                const unsigned int size = l_variations.size ();

                if ( itr != p_ndVtr.begin() )
                    l_totalPaths *= size;

                l_metaTree.push_back ( l_variations );
            }

            //qDebug() << "(ling) [Parser] Expanding across" << p_ndVtr.size () << "levels and expecting" << l_totalPaths << "different paths..." << endl;
            emit unwindingProgress(0.0);
            NodeTree l_tree = expandNodes ( l_metaTree , l_totalPaths , 0 );
            emit unwindingProgress(1.0);

            //qDebug() << "(ling) [Parser] Found" << l_tree.size() << "path(s)." << endl;

            emit finishedUnwinding();
            return l_tree;
        }

        /// @todo Determine a means of generating unique signatures.
        const string Parser::formShorthand ( const NodeVector& p_ndVtr, const Node::FormatDensity& p_sigVerb ) {
            string l_ndShrthnd;

            for ( NodeVector::const_iterator itr = p_ndVtr.begin (); itr != p_ndVtr.end (); ++itr ) {
                const FlatNode* l_nd = dynamic_cast<const FlatNode*>(*itr);
                l_ndShrthnd += l_nd->toString ( p_sigVerb );
            }

            return l_ndShrthnd;
        }

        /// @todo Find a meanings of collecting meanings.
        void Parser::parse ( const string& p_txt ) {
            QTextStream l_strm(p_txt.c_str (),QIODevice::ReadOnly);
            MeaningVector l_mngVtr;

            while (!l_strm.atEnd ()){
                QString l_str = l_strm.readLine ();
                Meaning* l_mng = const_cast<Meaning*>( process ( l_str.toStdString() ) );
                if (l_mng)
                    l_mngVtr.push_back (l_mng);

            }
        }

        /// @todo Obtain the one meaning that represents the entire parsed text.
        const Meaning* Parser::process ( const string& p_txt ) {
            StringVector l_tokens = getTokens ( p_txt );
            NodeVector l_theNodes = formNodes ( l_tokens );
            NodeTree l_nodeTree = expandNodes ( l_theNodes );

            MeaningVector l_meaningVtr;
            for ( NodeTree::const_iterator itr = l_nodeTree.begin (); itr != l_nodeTree.end (); itr++ ) {
                const NodeVector l_ndVtr = *itr;
                //qDebug() << "(ling) [Parser] " << "Forming meaning #" << (l_meaningVtr.size () + 1) << "..." << endl;
                Meaning* l_meaning = const_cast<Meaning*>(Meaning::form ( l_ndVtr ));
                if (l_meaning != NULL)
                    l_meaningVtr.push_back ( l_meaning );
            }

            unique ( l_meaningVtr.begin(),l_meaningVtr.end () );
            //qDebug() << "(ling) [Parser]" << l_nodeTree.size () << "paths formed" << l_meaningVtr.size () << "meanings." << endl;
            //cout << endl << setw(20) << setfill('=') << " " << endl;

            for ( MeaningVector::const_iterator itr2 = l_meaningVtr.begin (); itr2 != l_meaningVtr.end (); itr2++ ) {
                const Meaning* l_mngItr = *itr2;
                cout << l_mngItr->toText () << endl;
            }

            if (!l_meaningVtr.empty ()) return l_meaningVtr.front ();
            else return NULL;
        }

        const Meaning* Meaning::form ( const LinkVector* p_lnkVtr ) {
            return new Meaning ( const_cast<LinkVector*>(p_lnkVtr) );
        }

        /// @todo Raise an exception and crash if this word is foreign OR have it be registered under a psuedo word OR implement a means of creating a new RuleSet.
        /// @todo If a word fails the test, crash and explain there's a grammatical mistake OR add another rule if under educational mode.
        const Meaning* Meaning::form ( const NodeVector& p_ndVtr, LinkVector* p_lnkVtr ) {
            //cout << endl << setw(20) << setfill('=') << " " << endl;
            NodeVector::ConstIterator l_ndItr = p_ndVtr.begin ();
            NodeVector l_ndVtr;
            QStringList* l_hideList = NULL;
            bool l_hideOther = false, l_hideThis = false;
#if 0
            qDebug() << "(ling) [Meaning] Current nodes being parsed: '";
            for (int i = 0; i < p_ndVtr.size (); i++)
                qDebug() << p_ndVtr.at (i) << " ";
            qDebug() << "'" << endl;
#endif

            if (p_ndVtr.size () != 1){
                for ( ; l_ndItr != p_ndVtr.end (); l_ndItr++ ) {
                    const Node *l_nd, *l_nd2;
                    if ( p_ndVtr.size () == 2 ) {
                        l_nd = p_ndVtr.front ();
                        l_nd2 = p_ndVtr.back ();
                    } else {
                        if ( ( l_ndItr + 1 ) != p_ndVtr.end () ) {
                            l_nd =  ( * ( l_ndItr ) );
                            l_nd2 = ( * ( l_ndItr + 1 ) );
                        } else break;
                    }

                    if (l_hideList){
                        const QString l_k(l_nd->toString (Node::EXTRA).c_str ());
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

                    //qDebug() << "(ling) [Meaning] Current node: " << l_nd << endl;
                    const Binding* l_bnd = Binding::obtain ( *l_nd,*l_nd2 );
                    const Link* l_lnk;
                    if ( l_bnd ) {
                        l_lnk = l_bnd->bind ( *l_nd,*l_nd2 );
                        p_lnkVtr->push_back ( const_cast<Link*>(l_lnk) );

                        QString l_hide = l_bnd->getAttrValue("hide");
                        QString l_hideNext = l_bnd->getAttrValue("hideNext");
                        QString l_skipWord = l_bnd->getAttrValue("skipWord");
                        QString l_hideFilter = l_bnd->getAttrValue ("hideFilter");

                        l_hide = (l_hide.isEmpty () || l_hide.isNull ()) ? "no" : l_hide;
                        l_hideNext = (l_hideNext.isEmpty () || l_hideNext.isNull ()) ? "no" : l_hideNext;
                        l_skipWord = (l_skipWord.isEmpty () || l_skipWord.isNull ()) ? "yes" : l_skipWord;

                        // Attribute 'hide': Prevents this node (source node) from appearing on the next round of parsing. (default = 'no')
                        if ( !l_hideThis && !l_hideOther && l_hide == "no" )
                            l_ndVtr.push_back ( const_cast<Node*>( dynamic_cast<const Node*> ( l_lnk->source () ) ) );
                        else {
                            //qDebug() << "(ling) [Meaning] *** Hid '" << l_lnk->source () << "' from appearing on the next pass of parsing." << endl;
                        }

                        // Attribute 'hideNext': Prevents the next node (destination node) from appearing on its next round of parsing (2 rounds from now) (default = 'no')
                        if ( l_hideNext == "yes" ) {
                            l_hideOther = true;
                            //qDebug() << "(ling) [Meaning] *** Hid '" << l_lnk->destination () << "' from appearing on the next pass of parsing (2 rounds from now)." << endl;
                        } else l_hideOther = false;

                        // Attribute 'skipWord': Doesn't allow the destination node to have a chance at being parsed. (default = yes)
                        if ( l_skipWord == "yes" )
                            l_ndItr++;
                        else
                            qDebug() << "(ling) [Meaning] *** Skipping prevented for word-symbol '" << l_lnk->destination () << "'; will be parsed on next round." << endl;


                        // Attribute 'hideFilter': Hides a set of words from appearing on the next round of parsing; a wrapper for the 'hide' attribute. (default = "")
                        if ( l_hideFilter.length () != 0 ) {
                            QStringList *l_e = new QStringList;

                            if ( l_hideFilter.contains ( "," ) ) {
                                QStringList d = l_hideFilter.split ( "," );
                                foreach ( const QString q, d )
                                l_e->append ( q );
                            } else l_e->append ( l_hideFilter );

                            l_hideList = l_e;
                            //cout << "(ling) [Meaning] *** Hiding any nodes that falls into the regex '" << qPrintable(l_hideFilter) << "' on the next round." << endl;
                        }
#if 0
                        qDebug() << "(ling) [Meaning] Flags> hide: (" << l_hide
                             << ") hideThis: ("   << ((l_hideThis == true) ? "yes" : "no")
                             << ") hideOther: ("  << ((l_hideOther == true) ? "yes" : "no")
                             << ") hideNext: ("   << l_hideNext
                             << ") hideFilter: (" << l_hideFilter
                             << ") skipWord: ("   << l_skipWord
                             << ") hideList: ("   << ((l_hideList == NULL) ? "NULL" : "*") << ")"
                             << endl << "Link sig: " << l_lnk->toString ().c_str ()<< endl;
#endif

                    } else {
                        //qWarning() << "(ling) [Meaning] Linking failed ... horribly." << endl;
                        l_lnk = NULL;
                    }
#if 0
                    qDebug() << endl << "(ling) [Meaning] Nodes to be queued:";
                    for (int i = 0; i < l_ndVtr.size (); i++)
                        qDebug() << l_ndVtr.at (i);
                    qDebug() << "'" << endl << endl;
#endif

                }
                //qDebug() << "(ling) [Meaning] Formed" << p_lnkVtr->size () << "links with" << l_ndVtr.size () << "nodes left to parse." << endl << endl;
            }

            if ( !p_lnkVtr->empty () ) {
                if ( ! ( p_lnkVtr->size () >= 1) || l_ndVtr.size () > 0 )
                    return Meaning::form ( l_ndVtr , p_lnkVtr );
                else
                    return new Meaning ( p_lnkVtr );
            } else
                return NULL;
        }

        const Link* Meaning::base () const {
            if (m_lnkVtr == NULL) return NULL;
            return m_lnkVtr->back ();
        }
        const LinkVector* Meaning::siblings () const {
            return m_lnkVtr;
        }

        const LinkVector* Meaning::isLinkedTo(const Node& p_nd) const {
            LinkVector* l_lnkVtr = new LinkVector;

            for ( LinkVector::const_iterator i = m_lnkVtr->begin (); i != m_lnkVtr->end (); i++ ) {
                const Link* l_lnk = *i;
                if (l_lnk->source () == &p_nd)
                    l_lnkVtr->push_back (const_cast<Link*>(l_lnk));
            }

            if (l_lnkVtr->size () == 0) return NULL;
            else return l_lnkVtr;
        }

        const LinkVector* Meaning::isLinkedBy(const Node& p_nd) const {
            LinkVector* l_lnkVtr = new LinkVector;

            for ( LinkVector::const_iterator i = m_lnkVtr->begin (); i != m_lnkVtr->end (); i++ ) {
                const Link* l_lnk = *i;
                if (l_lnk->destination () == &p_nd)
                    l_lnkVtr->push_back (const_cast<Link*>(l_lnk));
            }

            if (l_lnkVtr->size () == 0) return NULL;
            else return l_lnkVtr;
        }

        const string Meaning::toText () const {
            const Link* l_lnk = m_lnkVtr->front ();
            cout << "(ling) [Meaning] This Meaning encapsulates " << m_lnkVtr->size () << " link(s)." << endl;

            for ( LinkVector::const_iterator i = m_lnkVtr->begin (); i != m_lnkVtr->end (); i++ ) {
                const Link* l_lnk = *i;
                cout << "(ling) [Meaning] " << l_lnk->source () << ") -> "
                     << l_lnk->destination () << endl;
            }

            cout << "(ling) [Meaning] Primary link: " << l_lnk->source ()->symbol () << " (" << l_lnk->source ()->toString ( Node::EXTRA ) << ") -> "
                 << l_lnk->destination ()->symbol () << " (" << l_lnk->destination ()->toString ( Node::EXTRA ) << ")" << endl;

            return " ";
        }
    }
}

// kate: indent-mode cstyle; space-indent on; indent-width 4;
