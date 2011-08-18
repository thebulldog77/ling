/**
 * @file    syntax.hpp
 * @author  Jacky Alcine <jackyalcine@gmail.com>
 * @date February 4, 2011, 12:07 AM
 * @license GPL3
 *
 * @legalese
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
/// @todo Replace the md5() feature and just have a giant, giant dictionary that links words to their IDs. In this case, every word (or a pre-defined list of words) should be loaded each time. Sounds like something MySQL would be perfect for.

#ifndef __SYNTAX_HPP__
#define __SYNTAX_HPP__

#include <string>
#include <syntax.hpp>
#include <QMap>
#include <QVector>
#include <wntrdata.hpp>

using namespace std;
using namespace Wintermute::Data::Linguistics;

using std::map;
using std::string;
using std::vector;
using Wintermute::Data::Linguistics::Lexical::Leximap;
using Wintermute::Data::Linguistics::Lexical::Lexidata;

namespace Wintermute {
    namespace Linguistics {
        struct Link;
        struct Node;
        struct FlatNode;

        typedef QVector<string> StringVector;
        typedef QVector<Node*> NodeVector;
        typedef QVector<Link*> LinkVector;

        class Node : public QObject {
            Q_GADGET
            Q_PROPERTY(const string id READ id)
            Q_PROPERTY(const string locale READ locale)
            Q_PROPERTY(const string symbol READ symbol)
            Q_PROPERTY(const string value READ toString)
            Q_PROPERTY(const Leximap flags READ flags)
            Q_PROPERTY(const Lexidata* data READ data)
            Q_ENUMS(FormatDensity)

            friend class FlatNode;
            protected:
                Node ( Lexidata p_lxdt ) : m_lxdt(p_lxdt) { }
                Lexidata m_lxdt;

            public:
                enum FormatDensity {
                    FULL = 0,
                    MINIMAL,
                    EXTRA
                };

                explicit Node( ) : m_lxdt() { }
                Node( const Node& p_nd ) : m_lxdt(p_nd.m_lxdt) {  }
                ~Node() { }

                Q_INVOKABLE inline const string id() const { return *(this->m_lxdt.id ()); }
                Q_INVOKABLE inline const string locale() const { return *(this->m_lxdt.locale ()); }
                Q_INVOKABLE inline const string symbol() const { return *(this->m_lxdt.symbol ()); }
                Q_INVOKABLE inline const Leximap flags() const { return this->m_lxdt.flags (); }
                Q_INVOKABLE inline const Lexidata* data() const { return &this->m_lxdt; }
                Q_INVOKABLE const string toString ( const FormatDensity& = FULL ) const;

                static const string toString ( const Node* , const FormatDensity& = FULL );
                static const string toString ( const NodeVector& , const FormatDensity& = FULL );
                static const Node* fromString ( const string& );
                static const Node* obtain ( const string&, const string& );
                static const Node* create( const Lexidata* );
                static const Node* buildPseudo ( const string&, const string&, const string& );
                static const bool exists ( const string&, const string& );

                bool operator == (const Node& p_nd){ return this->id () == p_nd.id () && this->m_lxdt.locale () == this->m_lxdt.locale () ;}
                QDebug operator<<(QDebug dbg) {
                     dbg.space () << this->symbol ().c_str () << " (" << this->toString (Node::EXTRA).c_str () << ")";
                     return dbg.space();
                }
        };

        class FlatNode : public Node {
            Q_OBJECT

            Q_PROPERTY(const char type READ type)

            protected:
                FlatNode ( const Node*, const int& );
                FlatNode ( const string&, const string&, const string&, const Leximap::value_type& );

            public:
                FlatNode();
                FlatNode ( const Node* p_nd ) : Node ( p_nd->m_lxdt ) {}
                FlatNode ( const FlatNode& p_nd ) : Node ( p_nd ) {}
                ~FlatNode();
                Q_INVOKABLE const char type() const;
                static const FlatNode* form ( const string&, const string&, const string&, const Leximap::value_type& );
                static const FlatNode* form ( const Node* , const int& = 0 );
                static NodeVector expand ( const Node* );
                QDebug operator<<(QDebug dbg) { return this->Node::operator << (dbg); }
        };

        class Link : public QObject {
            Q_OBJECT
            Q_PROPERTY(const FlatNode* source READ source)
            Q_PROPERTY(const FlatNode* destination READ destination)
            Q_PROPERTY(const string locale READ locale)
            Q_PROPERTY(const string flags READ flags)
            Q_PROPERTY(const string value READ toString)

            public:
                static const Link* form ( const FlatNode* , const FlatNode* , const string&, const string& );
                static const Link* fromString ( const string& );
                const FlatNode* source() const;
                const FlatNode* destination() const;
                const string locale() const;
                const string flags() const;
                const string toString() const;
                Link();
                Link(const Link& p_lnk ) : m_src(p_lnk.m_src), m_dst(p_lnk.m_dst),
                    m_flgs(p_lnk.m_flgs), m_lcl(p_lnk.m_lcl) { }
                QDebug operator<<(QDebug dbg) {
                     dbg.space () << this->toString ().c_str ();
                     return dbg.space();
                }

            protected:
                Link ( const FlatNode* , const FlatNode* , const string&, const string& );

            private:
                const FlatNode* m_src;
                const FlatNode* m_dst;
                const string m_flgs;
                const string m_lcl;
        };
    }
}

Q_DECLARE_METATYPE(Wintermute::Linguistics::Link)
Q_DECLARE_METATYPE(Wintermute::Linguistics::Node)
Q_DECLARE_METATYPE(Wintermute::Linguistics::FlatNode)

#endif	/* __SYNTAX_HPP */

// kate: indent-mode cstyle; space-indent on; indent-width 4;
