/*
 * The Doomsday Engine Project -- libdeng2
 *
 * Copyright (c) 2010-2012 Jaakko Keränen <jaakko.keranen@iki.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBDENG2_APP_H
#define LIBDENG2_APP_H

#include "../libdeng2.h"
#include "../CommandLine"
#include "../LogBuffer"
#include "../FS"
#include "../Module"
#include "../Config"
#include "../UnixInfo"
#include <QApplication>

/**
 * Macro for conveniently accessing the de::App singleton instance.
 */
#define DENG2_APP   (static_cast<de::App*>(qApp))

namespace de
{
    /**
     * Application whose event loop is protected against uncaught exceptions.
     * Catches the exception and shuts down the app cleanly.
     */
    class DENG2_PUBLIC App : public QApplication
    {
        Q_OBJECT

    public:
        /// The object or resource that was being looked for was not found. @ingroup errors
        DENG2_ERROR(NotFoundError);

        enum GUIMode {
            GUIDisabled = 0,
            GUIEnabled = 1
        };

        enum SubsystemInitFlag {
            DefaultSubsystems   = 0x0,
            DisablePlugins      = 0x1
        };
        Q_DECLARE_FLAGS(SubsystemInitFlags, SubsystemInitFlag)

    public:
        App(int& argc, char** argv, GUIMode guiMode);

        /**
         * Initializes all the application's subsystems. This includes Config
         * and FS. Has to be called manually in the application's
         * initialization routine.
         *
         * @param flags  How to/which subsystems to initialize.
         */
        void initSubsystems(SubsystemInitFlags flags = DefaultSubsystems);

        bool notify(QObject* receiver, QEvent* event);

        static App& app();

        /**
         * Returns the command line used to start the application.
         */
        static CommandLine& commandLine();

        /**
         * Returns the absolute native path of the application executable.
         */
        static String executablePath();

        /**
         * Returns the native path of the data base directory.
         */
        String nativeBasePath();

        /**
         * Returns the native path of where to load binaries (plugins).
         */
        String nativeBinaryPath();

        /**
         * Returns the native path where user-specific runtime files should be
         * placed. The user can override the location using the @em -userdir
         * command line option.
         */
        String nativeHomePath();

        /**
         * Returns the application's file system.
         */
        static FS& fileSystem();

        /**
         * Returns the root folder of the file system.
         */
        static Folder& rootFolder();

        /**
         * Returns the /home folder.
         */
        static Folder& homeFolder();

        /**
         * Returns the configuration.
         */
        static Config& config();

        /**
         * Returns the Unix system-level configuration preferences.
         */
        static UnixInfo& unixInfo();

        /**
         * Imports a script module that is located on the import path.
         *
         * @param name      Name of the module.
         * @param fromPath  Absolute path of the script doing the importing.
         *
         * @return  The imported module.
         */
        static Record& importModule(const String& name, const String& fromPath = "");

        /**
         * Emits the displayModeChanged() signal.
         *
         * @todo In the future when de::App (or a sub-object owned by it) is
         * responsible for display modes, this should be handled internally and
         * not via this public interface where anybody can call it.
         */
        void notifyDisplayModeChanged();

    signals:
        void uncaughtException(QString message);

        /// Emitted when the display mode has changed.
        void displayModeChanged();

    private:
        CommandLine _cmdLine;

        LogBuffer _logBuffer;

        /// Path of the application executable.
        String _appPath;

        /// The file system.
        FS _fs;

        UnixInfo _unixInfo;

        /// The configuration.
        Config* _config;

        /// Resident modules.
        typedef std::map<String, Module*> Modules;
        Modules _modules;
    };

    Q_DECLARE_OPERATORS_FOR_FLAGS(App::SubsystemInitFlags)
}

#endif // LIBDENG2_APP_H
