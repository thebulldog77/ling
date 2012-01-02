/**
 * @file    adaptors.cpp
 * @author  Wintermute Developers <wintermute-devel@lists.launchpad.net>
 *
 *
 *
 *
 * This is free software; you can redistribute it and/or modify
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

#include "adaptors.hpp"
#include "wntrling.hpp"
#include <QtDBus/QDBusConnection>

namespace Wintermute {
    namespace Linguistics {
        SystemAdaptor::SystemAdaptor() : Adaptor(Linguistics::System::instance()){
            QDBusConnection::sessionBus().connect ("org.thesii.Wintermute","/Master",
                                      "org.thesii.Wintermute.Master","aboutToQuit",
                                      this,SLOT(quit()));
            setAutoRelaySignals (true);
        }

        void SystemAdaptor::quit () const {
            Linguistics::System::stop ();
            CoreAdaptor::haltSystem ();
        }

        void SystemAdaptor::tellSystem (const QString &p_txt) const {
            Linguistics::System::tellSystem (p_txt);
        }
    }
}
