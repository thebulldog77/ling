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
 */

#include "syntax.hpp"
#include "parser.hpp"
#include <iostream>
#include <boost/tokenizer.hpp>
#include <QString>
#include <QFile>

using namespace boost;
using namespace std;

using std::cout;
using std::endl;

namespace Wintermute {
    namespace Linguistics {
        Parser::Parser() : m_lcl ( string() ) { }

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

            for ( tokenizer<>::const_iterator itr = tkn.begin ();
                    itr != tkn.end (); ++itr ) {
                const QString l_str ( ( *itr ).c_str() );
                l_theTokens.push_back ( l_str.toLower ().toStdString () );
            }

            return l_theTokens;
        }

        /// @todo Wrap the ID obtaining method into a private method by the parser.
        NodeVector Parser::formNodes ( StringVector& l_tokens ) {
            NodeVector l_theNodes;

            for ( StringVector::const_iterator itr = l_tokens.begin ();
                    itr != l_tokens.end (); ++itr ) {
                const string l_curToken = *itr;
                const string l_theID = md5 ( l_curToken );
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

            //cout << "(linguistics) [Parser] Level " << p_level << " should generate " << p_size << " paths." << endl;
            const NodeVector l_curBranch = p_tree.at ( p_level );

            const bool isAtEnd = ( p_level + 1 == p_tree.size () );

            if ( l_curBranch.empty () ) {
                cout << "(linguistics) [Parser] WARNING! Invalid level detected at level " << p_level << "." << endl;
                return ( NodeTree() );
            }

            //cout << "(linguistics) [Parser] Level " << p_level << " has " << l_curBranch.size() << " variations." << endl;

            const int l_mxSize = p_size / l_curBranch.size ( );

            NodeTree l_chldBranches, l_foundStems = expandNodes ( p_tree , l_mxSize , p_level + 1 );

            //cout << "(linguistics) [Parser] Level " << p_level << " expects " << l_foundStems.size() * l_curBranch.size () << " paths." << endl;
            // iterate upon current level.
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

            //cout << "(linguistics) [Parser] Level " << p_level << " generated " << l_chldBranches.size() << " branches." << endl;
            // Return that.
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

                if ( size >= 1 )
                    l_totalPaths *= size;

                l_metaTree.push_back ( l_variations );
            }

            //cout << "(linguistics) [Parser] Expanding across " << p_nodVtr.size () << " levels and expecting " << l_totalPaths << " different paths..." << endl;
            NodeTree l_tree = expandNodes ( l_metaTree , l_totalPaths , 0 );

            //cout << "(linguistics) [Parser] Found " << l_tree.size() << " paths." << endl;

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
            for (NodeTree::const_iterator itr = l_nodeTree.begin (); itr != l_nodeTree.end (); itr++){
                const NodeVector l_ndVtr = *itr;
                const Meaning* l_meaning = Meaning::form(l_ndVtr);
                l_meaningVtr.push_back (l_meaning);
            }

            unique(l_meaningVtr.begin(),l_meaningVtr.end ());

            cout << l_nodeTree.size () << " paths." << endl;
        }

        Meaning::Meaning() : m_lnk(NULL), m_lnkVtr(NULL) { }

        Meaning::Meaning(const Link* p_lnk, const LinkVector* p_lnkVtr) : m_lnk(p_lnk), m_lnkVtr(p_lnkVtr) {}

        /// @todo Add a means of optimizing here..
        const Meaning* Meaning::form(const Link* p_lnk, const LinkVector* p_lnkVtr){
            return new Meaning(p_lnk,p_lnkVtr);
        }

        /// @todo Convert a list of nodes into a Meaning object.
        /// @todo Implement a means of determining a link from a NodeVector.

        const Meaning* Meaning::form(const NodeVector& p_ndVtr){
            Link* l_rtLnk = NULL;
            LinkVector* l_rtLnkVtr = new LinkVector;

            // Form signature of vector.
            const string l_sig = Node::toString (p_ndVtr);
            cout << l_sig << endl;

            // Look for familiar links or sub-links so we can reduce the size of this vector.

            // Now, with it reduced to the most simplest form, form the root link.

            // Form the meaning and return it.
            return new Meaning(l_rtLnk,l_rtLnkVtr);
        }

        const Link* Meaning::base () const { return m_lnk; }
        const LinkVector* Meaning::siblings () const { return m_lnkVtr; }
    }
}

// kate: indent-mode cstyle; space-indent on; indent-width 4;


