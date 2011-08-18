/*
 * wntrling.cpp
 * This file is part of Wintermute Linguistics
 *
 * Copyright (C) 2011 - Jacky Alcine
 *
 * Wintermute Linguistics is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Wintermute Linguistics is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Wintermute Linguistics; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "wntrling.hpp"
#include <string>
#include <QVector>
#include <boost/python.hpp>
#include <boost/smart_ptr.hpp>

using namespace std;
using namespace boost::python;
using namespace Wintermute::Linguistics;

using std::string;
using boost::python::class_;

extern "C" {
    const string Node_toString(const Wintermute::Linguistics::Node* p_1){
        return p_1->toString();
    }

    /// @bug Is this the approriate way to go about this?
    const boost::shared_ptr<Wintermute::Linguistics::Node> Node_obtain(const string p_1, const string p_2){
        return boost::shared_ptr<Wintermute::Linguistics::Node>(const_cast<Wintermute::Linguistics::Node*>(Wintermute::Linguistics::Node::obtain (p_1,p_2)));
    }

    const boost::shared_ptr<Wintermute::Linguistics::Node> Node_create(const Wintermute::Data::Linguistics::Lexical::Lexidata* p_1){
        return boost::shared_ptr<Wintermute::Linguistics::Node>(const_cast<Wintermute::Linguistics::Node*>(Wintermute::Linguistics::Node::create (p_1)));
    }

    const boost::shared_ptr<Wintermute::Linguistics::Node> Node_buildPseudo(const string p_1, const string p_2, const string p_3){
        return boost::shared_ptr<Wintermute::Linguistics::Node>(const_cast<Wintermute::Linguistics::Node*>(Wintermute::Linguistics::Node::buildPseudo (p_1,p_2,p_3)));
    }

    const boost::shared_ptr<FlatNode> Link_source(const Link* p_lnk){
        return boost::shared_ptr<FlatNode>(const_cast<FlatNode*>(p_lnk->source ()));
    }

    const boost::shared_ptr<FlatNode> Link_destination(const Link* p_lnk){
        return boost::shared_ptr<FlatNode>(const_cast<FlatNode*>(p_lnk->destination ()));
    }
}

BOOST_PYTHON_MODULE(wntrling){

    class_<NodeVector>("NodeVector",init< NodeVector >());

    class_<Wintermute::Linguistics::Parser, boost::noncopyable>("Parser",init< string >())
            .add_property("locale",&Wintermute::Linguistics::Parser::locale,&Wintermute::Linguistics::Parser::setLocale)
            .def("parse",&Wintermute::Linguistics::Parser::parse)
    ;

    class_<Wintermute::Linguistics::Node, boost::noncopyable>("Node", no_init)
            .add_property("symbol",&Wintermute::Linguistics::Node::symbol)
            .add_property("id",&Wintermute::Linguistics::Node::id)
            .add_property("locale",&Wintermute::Linguistics::Node::locale)
            //.def("toString",&Wintermute::Linguistics::Node::toString)
            .def("exists",&Wintermute::Linguistics::Node::exists)
            .def("obtain",&Node_obtain)
            .def("create",&Node_create)
            .def("buildPseudo",&Node_buildPseudo)
    ;

    class_<FlatNode, bases<Wintermute::Linguistics::Node>, boost::noncopyable >("FlatNode", no_init)
            .def("expand",&FlatNode::expand)
    ;

    class_<Link, boost::noncopyable>("Link", no_init)
            .add_property("source",&Link_source)
            .add_property("destination",&Link_destination)
            .add_property("locale",&Link::locale)
            .def("toString",&Link::toString)
    ;

    class_<Meaning, boost::noncopyable>("Meaning",no_init)
            //.add_property("base",&Meaning::base)
            //.add_property("siblings",&Meaning::siblings)
            //.def("toText",&Meaning::toText)
    ;
}


// kate: indent-mode cstyle; space-indent on; indent-width 4;
