/**
 * @file    syntax.hpp
 * @author  Wintermute Developers <wintermute-devel@lists.launchpad.net>
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

#ifndef __SYNTAX_HPP__
#define __SYNTAX_HPP__

#include <string>
#include <syntax.hpp>
#include <QMap>
#include <QList>
#include <wntrdata.hpp>

using namespace std;
using namespace Wintermute::Data::Linguistics;

using std::string;
using Wintermute::Data::Linguistics::Lexical::Data;
using Wintermute::Data::Linguistics::Lexical::DataFlagMap;

namespace Wintermute {
    namespace Linguistics {
        struct Link;
        struct Node;
        struct Binding;

        /**
         * @brief Represents a @c QVector of strings.
         * @typedef StringVector
         */
        typedef QList<string> StringVector;
        /**
         * @brief Represents a @c QVector of @c Nodes;
         * @typedef NodeVector
         */
        typedef QList<Node*> NodeVector;
        /**
         * @brief Represents a @c QVector of @c Links;
         * @typedef LinkVector
         */
        typedef QList<Link*> LinkVector;

        /**
         * @brief An object representing the lexical and syntactic bindings of a word-symbol.
         * Wintermute's natural language processing system uses nodes to represent individual word
         * symbols in lexical representation and in syntactical representation.
         * @class Node syntax.hpp "include/wntr/ling/syntax.hpp"
         */
        class Node : public QObject {
            Q_GADGET
            Q_PROPERTY(const QString id READ id)
            Q_PROPERTY(const QString locale READ locale)
            Q_PROPERTY(const QString symbol READ symbol)
            Q_PROPERTY(const QString value READ toString)
            Q_PROPERTY(const DataFlagMap flags READ flags)
            Q_PROPERTY(const Data* data READ data)
            Q_ENUMS(FormatVerbosity)

            friend class Link;

            protected:
                Lexical::Data m_lxdt;

            public:
                /**
                 * @brief The format verbosity of a Node in text.
                 * The enumeration shown here represents the verbosity of the information
                 * generated when representing a @c Node as a string.
                 * @enum FormatVerbosity.
                 */
                enum FormatVerbosity {
                    FULL = 0, /**< TODO */
                    MINIMAL, /**< TODO */
                    EXTRA /**< TODO */
                };

                /**
                 * @brief Null constructor.
                 * @fn Node
                 */
                Node( ) : m_lxdt() { }

                /**
                 * @brief
                 * @fn Node
                 * @param p_lxdt
                 */
                Node ( Lexical::Data p_lxdt ) : m_lxdt(p_lxdt) {
                    this->setProperty ("OriginalToken",symbol ());
                }
                /**
                 * @brief
                 * @fn Node
                 * @param p_nd
                 */
                Node( const Node& p_nd ) : m_lxdt(p_nd.m_lxdt) {
                    this->setProperty ("OriginalToken",symbol ());
                }
                /**
                 * @brief
                 * @fn ~Node
                 */
                ~Node() { }

                /**
                 * @brief
                 * @fn id
                 * @return const string
                 */
                Q_INVOKABLE inline const QString id() const { return m_lxdt.id (); }
                /**
                 * @brief
                 * @fn locale
                 * @return const string
                 */
                Q_INVOKABLE inline const QString locale() const { return m_lxdt.locale (); }
                /**
                 * @brief
                 * @fn symbol
                 * @return const string
                 */
                Q_INVOKABLE inline const QString symbol() const { return m_lxdt.symbol (); }
                /**
                 * @brief
                 * @fn flags
                 * @return const DataFlagMap
                 */
                Q_INVOKABLE inline const DataFlagMap flags() const { return m_lxdt.flags (); }
                /**
                 * @brief
                 * @fn data
                 * @return const Data *
                 */
                Q_INVOKABLE inline const Lexical::Data* data() const { return &m_lxdt; }
                /**
                 * @brief
                 * @fn isFlat
                 * @return const bool
                 */
                Q_INVOKABLE inline const bool isFlat() const { return this->flags ().size () == 1; }
                /**
                 * @brief
                 *
                 * @fn isPseudo
                 * @return const bool
                 */
                Q_INVOKABLE inline const bool isPseudo() const { return Lexical::Cache::isPseudo (m_lxdt); }
                /**
                 * @brief
                 * @fn toString
                 * @param
                 */
                Q_INVOKABLE const string toString ( const FormatVerbosity& = FULL ) const;

                /**
                 * @brief
                 * @fn toString
                 * @param
                 * @param
                 */
                static const string toString ( const Node* , const FormatVerbosity& = FULL );
                /**
                 * @brief
                 * @fn toString
                 * @param
                 * @param
                 */
                static const string toString ( const NodeVector& , const FormatVerbosity& = FULL );
                /**
                 * @brief
                 * @fn exists
                 * @param
                 * @param
                 */
                static const bool exists ( const string&, const string& );
                /**
                 * @brief
                 * @fn obtain
                 * @param
                 * @param
                 */
                static const Node* obtain ( const string&, const string& );
                /**
                 * @brief
                 * @fn create
                 * @param
                 */
                static const Node* create( const Lexical::Data& );
                /**
                 * @brief
                 * @fn fromString
                 * @param
                 */
                static const Node* fromString ( const string& );
                /**
                 * @brief
                 * @fn buildPseudo
                 * @param
                 * @param
                 */
                static const Node* buildPseudo ( const string&, const string& );

                /**
                 * @brief
                 *
                 * @fn form
                 * @param
                 */
                static const Node* form ( Lexical::Data );

                /**
                 * @brief
                 * @fn expand
                 * @param
                 */
                static NodeVector expand ( const Node* );

                /**
                 * @brief
                 * @fn operator==
                 * @param p_nd
                 * @return bool
                 */
                bool operator== (const Node& p_nd){
                    return this->id () == p_nd.id () &&
                           this->m_lxdt.locale () == this->m_lxdt.locale ();
                }

        };

        /**
         * @brief Represents syntactical binding of two nodes.
         * The binding object that describes the relationship between two nodes can be found
         * in this class. The @c Link object demonstrates the syntactic connection between two
         * nodes.
         * @class Link syntax.hpp "include/wntr/ling/syntax.hpp"
         */
        class Link : public QObject {
            Q_OBJECT
            Q_PROPERTY(const Node* source READ source)
            Q_PROPERTY(const Node* destination READ destination)
            Q_PROPERTY(const string locale READ locale)
            Q_PROPERTY(const string flags READ flags)
            Q_PROPERTY(const string value READ toString)

            public:
                /**
                 * @brief
                 * @fn form
                 * @param
                 * @param
                 * @param
                 * @param
                 */
                static const Link* form ( const Node* , const Node* , const string&, const string& );
                /**
                 * @brief
                 * @fn fromString
                 * @param
                 */
                static const Link* fromString ( const string& );
                /**
                 * @brief
                 * @fn source
                 * @return const Node *
                 */
                Q_INVOKABLE inline const Node* source() const { return m_src; }
                /**
                 * @brief
                 * @fn destination
                 * @return const Node *
                 */
                Q_INVOKABLE inline const Node* destination() const { return m_dst; }
                /**
                 * @brief
                 * @fn locale
                 * @return const string
                 */
                Q_INVOKABLE inline const string locale() const { return m_lcl; }
                /**
                 * @brief
                 * @fn flags
                 * @return const string
                 */
                Q_INVOKABLE inline const string flags() const { return m_flgs; }
                /**
                 * @brief
                 * @fn toString
                 */
                Q_INVOKABLE const string toString() const;

                /**
                 * @brief
                 * @fn Link
                 */
                Link() : m_lcl(""), m_flgs(""), m_src(NULL), m_dst(NULL){ }
                /**
                 * @brief
                 * @fn Link
                 * @param p_lnk
                 */
                Link(const Link& p_lnk ) : m_src(p_lnk.m_src), m_dst(p_lnk.m_dst), m_flgs(p_lnk.m_flgs), m_lcl(p_lnk.m_lcl) { }

            protected:
                /**
                 * @brief
                 * @fn Link
                 * @param p_src
                 * @param p_dst
                 * @param p_flgs
                 * @param p_lcl
                 */
                Link ( const Node* p_src, const Node* p_dst, const string& p_flgs, const string& p_lcl ) :
                    m_src ( p_src ),m_dst ( p_dst ), m_flgs ( p_flgs ), m_lcl ( p_lcl ) { }

            private:
                const Node* m_src;
                const Node* m_dst;
                const string m_flgs;
                const string m_lcl;
        };

        /**
         * @brief
         * @fn operator <<
         * @param QDebug
         * @param
         */
        QDebug operator<<(QDebug , const Node *);
        /**
         * @brief
         * @fn operator <<
         * @param QDebug
         * @param
         */
        QDebug operator<<(QDebug , const Link *);
    }
}

Q_DECLARE_METATYPE(Wintermute::Linguistics::Link)
Q_DECLARE_METATYPE(Wintermute::Linguistics::Node)

#endif	/* __SYNTAX_HPP */

// kate: indent-mode cstyle; space-indent on; indent-width 4;
