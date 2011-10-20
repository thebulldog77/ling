/**
 * @file wntrling.cpp
 * Copyright (C) 2011 - Wintermute Developers <wintermute-devel@lists.launchpad.net>
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

namespace Wintermute {
    namespace Linguistics {
        System* System::s_sys = NULL;

        System::System() : QObject(), m_prs((new Parser)) {
            if (System::s_sys)
                System::s_sys->deleteLater ();

            System::s_sys = this;
        }

        System::~System () { m_prs->deleteLater (); }

        System* System::instance () {
            if (!System::s_sys)
                System::s_sys = new System;

            return s_sys;
        }

        void System::start () { emit s_sys->started (); }

        void System::stop () { emit s_sys->stopped (); }

        void System::tellSystem (const QString &p_txt){
            s_sys->m_prs->parse (p_txt);
            emit s_sys->responseFormed(p_txt);
        }

        void Plugin::initialize () const {
             Linguistics::SystemAdaptor* l_adpt = new Linguistics::SystemAdaptor;
             Wintermute::IPC::System::registerObject ("/System", l_adpt);
        }

        void Plugin::deinitialize () const {
        }

        QObject* Plugin::instance () const { return System::instance(); }

    }
}

Q_EXPORT_PLUGIN2(WntrLing, Wintermute::Linguistics::Plugin)
// kate: indent-mode cstyle; space-indent on; indent-width 4;
