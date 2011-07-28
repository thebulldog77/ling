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

#ifndef __SYNTAX_HPP__
#define __SYNTAX_HPP__

#include <string>
#include <wntr/data/wntrdata.hpp>

using namespace std;
using namespace Wintermute::Data::Linguistics;

using std::vector;
using std::map;
using std::string;
using Wintermute::Data::Linguistics::Lexidata;

namespace Wintermute {
	namespace Linguistics {
		struct Link;
		struct Node;
		struct FlatNode;

		/**
		 * @brief
		 *
		 * @typedef StringCollection
		 */
		typedef map<const string, const string> StringCollection;
		/**
		 * @brief
		 *
		 * @typedef StringVector
		 */
		typedef vector<string> StringVector;
		/**
		 * @brief
		 *
		 * @typedef NodeVector
		 */
		typedef vector<Node> NodeVector;

		/**
		 * @brief
		 *
		 * @typedef LinkVector
		 */
		typedef vector<const Link*> LinkVector;
		/**
		 * @brief
		 *
		 * @typedef RootLink
		 */
		typedef pair<const Link*, const LinkVector> RootLink;

		/**
		 * @brief
		 *
		 * @class Node syntax.hpp "include/wntr/ling/syntax.hpp"
		 */
		class Node {
			friend class FlatNode;
		protected:
			/**
			 * @brief
			 *
			 * @fn Node
			 * @param
			 */
			Node(Lexidata* );
			Lexidata* m_lxdt;

		public:
			/**
			 * @brief
			 *
			 * @fn Node
			 */
			Node();
			/**
			 * @brief
			 * @enum FormatDensity
			 */
			enum FormatDensity {
				FULL = 0,
				MINIMAL,
				EXTRA
			};

			~Node();
			/**
			 * @brief
			 *
			 * @fn getID
			 */
			const string id() const;
			/**
			 * @brief
			 *
			 * @fn getLocale
			 */
			const string locale() const;
			/**
			 * @brief
			 *
			 * @fn getSymbol
			 */
			const string symbol() const;
			/**
			 * @brief
			 *
			 * @fn getFlags
			 */
			const Leximap* flags() const;
			/**
			 * @brief
			 *
			 * @fn toString
			 */
			const string toString(const FormatDensity& = FULL) const;
			static const string toString(const Node& , const FormatDensity& = FULL);
			static const string toString(const NodeVector& , const FormatDensity& = FULL);
			static const Node* fromString(const string& );
			/**
			 * @brief
			 *
			 * @fn obtain
			 * @param
			 * @param
			 */
			static const Node* obtain(const string&, const string& );
			/**
			 * @brief
			 *
			 * @fn buildPseudo
			 * @param
			 * @param
			 * @param
			 */
			static const Node* buildPseudo(const string&, const string&, const string& );
			/**
			 * @brief
			 *
			 * @fn exists
			 * @param
			 * @param
			 */
			static const bool exists(const string&, const string& );
		};

		/**
		 * @brief
		 *
		 * @class FlatNode syntax.hpp "include/wntr/ling/syntax.hpp"
		 */
		class FlatNode : public Node {
		protected:
			/**
			 * @brief
			 *
			 * @fn FlatNode
			 * @param
			 * @param
			 */
			FlatNode(const Node&, const int&);
			/**
			 * @brief
			 *
			 * @fn FlatNode
			 * @param
			 * @param
			 * @param
			 * @param
			 */
			FlatNode(const string&, const string&,
					 const string&, const StringCollection::value_type& );

		public:
			/**
			 * @brief
			 *
			 * @fn FlatNode
			 */
			FlatNode();
			/**
			 * @brief
			 *
			 * @fn FlatNode
			 * @param p_nd
			 */
			FlatNode(const Node& p_nd) : Node(p_nd.m_lxdt) {}
			~FlatNode();
			/**
			 * @brief
			 *
			 * @fn getType
			 */
			const char type() const;
			/**
			 * @brief
			 *
			 * @fn form
			 * @param
			 * @param
			 * @param
			 * @param
			 */
			static const FlatNode* form(const string&, const string&, const string&, const StringCollection::value_type& );
			/**
			 * @brief
			 *
			 * @fn form
			 * @param
			 * @param
			 */
			static const FlatNode* form(const Node& , const int& = 0);
			/**
			 * @brief
			 *
			 * @fn expand
			 * @param
			 */
			static NodeVector expand(const Node& );
		};

		/**
		 * @brief
		 *
		 * @class Link syntax.hpp "include/wntr/ling/syntax.hpp"
		 */
		class Link {
		public:
			/**
			 * @brief
			 *
			 * @fn form
			 * @param
			 * @param
			 * @param
			 * @param
			 */
			static const Link* form(const FlatNode* , const FlatNode* , const string&, const string&);
			/**
			 * @brief
			 *
			 * @fn fromString
			 * @param
			 */
			static const Link* fromString(const string&);
			/**
			 * @brief
			 *
			 * @fn source
			 */
			const FlatNode* source() const;
			/**
			 * @brief
			 *
			 * @fn destination
			 */
			const FlatNode* destination() const;
			/**
			 * @brief
			 *
			 * @fn locale
			 */
			const string locale() const;
			/**
			 * @brief
			 *
			 * @fn toString
			 */
			const string toString() const;

		protected:
			/**
			 * @brief
			 *
			 * @fn Link
			 * @param
			 * @param
			 * @param
			 * @param
			 */
			Link(const FlatNode* , const FlatNode* , const string&, const string&);
			/**
			 * @brief
			 *
			 * @fn Link
			 */
			Link();

		private:
			const FlatNode m_src;
			const FlatNode m_dst;
			const string m_flgs;
			const string m_lcl;
		};
	}
}
#endif	/* __SYNTAX_HPP__ */





