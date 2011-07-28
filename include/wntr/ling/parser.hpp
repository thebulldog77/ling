/**
 * @file    parser.cpp
 * @author  Jacky Alcine <jackyalcine@gmail.com>
 * @date    June 14, 2011 11:34 PM
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

#include <string>
#include <QString>
#include <wntr/data/md5.hpp>
#include <syntax.hpp>

using namespace std;

using std::string;

namespace Wintermute {
    namespace Linguistics {
        struct Parser;
        struct Meaning;

        /**
         * @brief Represents a vector of vector of nodes.
         * @typedef NodeTree
         */
        typedef vector<NodeVector> NodeTree;

		/**
		 * @brief Represents a collection of meanings.
		 * @typedef MeaningVector;
		 */
		typedef vector<const Meaning*> MeaningVector;

        /**
         * @brief
         * @class Parser parser.hpp "include/wntr/ling/parser.hpp"
         */
        class Parser {
        public:

            /**
             * @brief
             * @fn Parser
             * @param
             */
            Parser(const string& );
            /**
             * @brief
             * @fn getLocale
             */
            const string locale() const;
            /**
             * @brief
             * @fn setLocale
             * @param
             */
            void setLocale(const string& );
            /**
             * @brief
             * @fn parse
             * @param
             */
            void parse(const string& );
            /**
             * @brief
             * @fn process
             * @param
             */
            void process(const string& );

        protected:
            /**
             * @brief
             * @fn Parser
             */
            Parser();
            mutable string m_lcl;

        private:
            /**
             * @brief
             * @fn getTokens
             * @param
             */
            StringVector getTokens(const string& );
            /**
             * @brief
             * @fn formNodes
             * @param
             */
            NodeVector formNodes(StringVector& );
            /**
             * @brief
             * @fn expandNodes
             * @param
             */
            NodeTree expandNodes(NodeVector& );
            /**
             * @brief
             * @fn expandNodes
             * @param
             * @param
             * @param
             */
            NodeTree expandNodes(NodeTree& , const int& = 0, const int& = 0);
            /**
             * @brief
             *
             * @fn formMeaning
             * @param
             */
            const Meaning formMeaning(const NodeVector& );
            /**
             * @brief
             * @fn formShorthand
             * @param
             * @param
             */
            static const string formShorthand(const NodeVector& , const Node::FormatDensity& = Node::FULL );
        };

        class Meaning {
        public:
            const Link* base() const;
            const LinkVector* siblings() const;
            static const Meaning* form(const NodeVector& );
            static const Meaning* form(const Link* = NULL, const LinkVector* = NULL);

		protected:
			Meaning();
			Meaning(const Link* = NULL, const LinkVector* = NULL);

        private:
            const Link* m_lnk;
            const LinkVector* m_lnkVtr;
        };
    }
}
