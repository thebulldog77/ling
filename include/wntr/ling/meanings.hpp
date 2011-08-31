/**
 * @file    meanings.hpp
 * @author  Wintermute Developers <wintermute-devel@lists.launchpad.net>
 * @date    August 20, 2011 8:54 PM
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

#ifndef __MEANINGS_HPP__
#define __MEANINGS_HPP__

#include <QObject>
#include <QMultiMap>
#include <syntax.hpp>

namespace Wintermute {
    namespace Linguistics {
        struct Meaning;

        /**
         * @brief
         * @typedef OntoMap
         */
        typedef QMultiMap<Node*,Link*> OntoMap;

        /**
         * @brief Represents the end-result of linguistics linking and provides an easier means of glancing into the linking process.
         * @class Meaning parser.hpp "include/wntr/ling/parser.hpp"
         */
        class Meaning : public QObject {
            Q_OBJECT

            Q_PROPERTY(const Link* base READ base)
            Q_PROPERTY(const LinkVector* siblings READ siblings)

            public:
                /**
                 * @brief
                 *
                 * @fn Meaning
                 */
                Meaning();
                /**
                 * @brief
                 * @fn Meaning
                 * @param p_lnkVtr
                 */
                explicit Meaning(const LinkVector&);

                /**
                 * @brief
                 * @fn Meaning
                 * @param p_mng
                 */
                Meaning(const Meaning&);

                /**
                 * @brief
                 * @fn ~Meaning
                 */
                ~Meaning();

                /**
                 * @brief
                 * @fn base
                 */
                Q_INVOKABLE const Link* base() const;

                /**
                 * @brief
                 * @fn siblings
                 * @return const LinkVector *
                 */
                Q_INVOKABLE const LinkVector* siblings() const;

                /**
                 * @brief
                 * @fn toText
                 */
                const QString toText() const;

                /**
                 * @brief
                 * @fn isLinkedTo
                 * @param
                 */
                const LinkVector isLinkedTo(const Node& ) const;

                /**
                 * @brief
                 * @fn isLinkedBy
                 * @param
                 */
                const LinkVector isLinkedBy(const Node& ) const;

                /**
                 * @brief
                 *
                 * @fn form
                 * @param
                 * @param
                 */
                static const Meaning* form ( LinkVector* , const NodeVector& );

            protected:
                LinkVector m_lnkVtr;
                OntoMap m_ontoMap;

            private:
                void __init();
        };
    }
}

#endif

Q_DECLARE_METATYPE(Wintermute::Linguistics::Meaning)
