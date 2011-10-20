/**
 * @file wntrling.hpp
 * This file is part of Wintermute Linguistics
 *
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
 *
 *
 * @mainpage Wintermute Linguistics (WntrLing)
 * Wintermute uses an unique blend of natural language processing learning utilities,
 * some of which are similar to already existing projects (such as LinkGrammar at OpenCog
 * or Panlingua by Chamount Devin). The natural language processing used here is an adaptive,
 * link-based approach. By using an externally defined set of rules and an external list
 * of words with each of its tenses and variations linked to an ontology concept; it's
 * possible for WntrLing to form ontological representation of natural text. This, in turn,
 * provides an immediate form of natural language processing.D
 *
 */

#ifndef WINTERMUTE_LINGUISTICS
#define WINTERMUTE_LINGUISTICS

#include "syntax.hpp"
#include "parser.hpp"
#include "meanings.hpp"
#include "adaptors.hpp"
#include <wntr/plugins.hpp>
#include <QtPlugin>
#include <QObject>

using Wintermute::Plugin::AbstractPlugin;

namespace Wintermute {
    namespace Linguistics {
        struct System;
        struct Plugin;

        /**
         * @brief
         *
         * @class System wntrling.hpp "src/wntrling.hpp"
         */
        class System : public QObject {
            Q_OBJECT
            Q_DISABLE_COPY(System)

            private:
                static System* s_sys; /**< The internal instance. */
                Parser* m_prs; /**< The global parser instance. */
                /**
                 * @brief
                 *
                 * @fn System
                 */
                System();
                ~System();

            signals:
                /**
                 * @brief
                 *
                 * @fn started
                 */
                void started();

                /**
                 * @brief
                 *
                 * @fn stopped
                 */
                void stopped();

                /**
                 * @brief
                 *
                 * @fn responseFormed
                 * @param
                 */
                void responseFormed(const QString&);

            public:
                /**
                 * @brief
                 *
                 * @fn instance
                 */
                static System* instance();

            public slots:
                /**
                 * @brief
                 *
                 * @fn start
                 */
                static void start();

                /**
                 * @brief
                 *
                 * @fn stop
                 */
                static void stop();

                /**
                 * @brief
                 *
                 * @fn tellSystem
                 * @param
                 */
                static void tellSystem(const QString& );
        };

       class Plugin : public AbstractPlugin {
            Q_OBJECT
            public:
                Plugin() : AbstractPlugin() { }
                ~Plugin() { }
                Plugin(Plugin const &k) : AbstractPlugin(k) { }

                virtual void initialize() const;
                virtual void deinitialize() const;
                virtual QObject* instance() const;
        };
    }
}

#endif /* WINTERMUTE_LINGUISTICS */
// kate: indent-mode cstyle; space-indent on; indent-width 4;
