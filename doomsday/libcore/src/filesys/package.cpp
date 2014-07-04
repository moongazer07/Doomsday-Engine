/** @file package.cpp  Package containing metadata, data, and/or files.
 *
 * @authors Copyright (c) 2014 Jaakko Keränen <jaakko.keranen@iki.fi>
 *
 * @par License
 * LGPL: http://www.gnu.org/licenses/lgpl.html
 *
 * <small>This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version. This program is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
 * General Public License for more details. You should have received a copy of
 * the GNU Lesser General Public License along with this program; if not, see:
 * http://www.gnu.org/licenses</small>
 */

#include "de/Package"
#include "de/PackageLoader"
#include "de/Process"
#include "de/Script"
#include "de/ScriptedInfo"
#include "de/TimeValue"
#include "de/App"

namespace de {

static String const PACKAGE("package");

DENG2_PIMPL(Package)
, DENG2_OBSERVES(File, Deletion)
{
    File const *file;

    Instance(Public *i, File const *f)
        : Base(i)
        , file(f)
    {
        if(file) file->audienceForDeletion() += this;
    }

    ~Instance()
    {
        if(file) file->audienceForDeletion() -= this;
    }

    void fileBeingDeleted(File const &)
    {
        file = 0;
    }

    void verifyFile() const
    {
        if(!file)
        {
            throw SourceError("Package::verify", "Package's source file missing");
        }
    }
};

Package::Package(File const &file) : d(new Instance(this, &file))
{}

Package::~Package()
{}

File const &Package::file() const
{
    d->verifyFile();
    return *d->file;
}

Folder const &Package::root() const
{
    d->verifyFile();
    return d->file->as<Folder>();
}

Record &Package::info()
{
    d->verifyFile();
    return const_cast<File *>(d->file)->info();
}

Record const &Package::info() const
{
    return const_cast<Package *>(this)->info();
}

String Package::identifier() const
{
    d->verifyFile();
    return identifierForFile(*d->file);
}

bool Package::executeFunction(String const &name)
{
    QString const funcName = PACKAGE + "." + name;

    if(info().has(funcName))
    {
        Script script(name + "()");
        // The global namespace for this function is the package's info namespace.
        Process proc(&info().subrecord(PACKAGE));
        proc.run(script);
        proc.execute();
        return true;
    }
    return false;
}

void Package::didLoad()
{
    executeFunction("onLoad");
}

void Package::aboutToUnload()
{
    executeFunction("onUnload");
}

void Package::parseMetadata(File &packageFile) // static
{
    static String const TIMESTAMP("__timestamp__");

    if(Folder *folder = packageFile.maybeAs<Folder>())
    {
        // The package's information is stored in a subrecord.
        if(!packageFile.info().has(PACKAGE))
        {
            packageFile.info().addRecord(PACKAGE);
        }

        Record &metadata        = packageFile.info().subrecord(PACKAGE);
        File *metadataInfo      = folder->tryLocateFile("Info");
        File *initializerScript = folder->tryLocateFile("__init__.de");
        Time parsedAt           = Time::invalidTime();
        bool needParse          = true;

        if(!metadataInfo && !initializerScript) return; // Nothing to do.

        if(metadata.has(TIMESTAMP))
        {
            // Already parsed.
            needParse = false;

            // Only parse if the source has changed.
            if(TimeValue const *time = metadata.get(TIMESTAMP).maybeAs<TimeValue>())
            {
                needParse =
                        (metadataInfo      && metadataInfo->status().modifiedAt      > time->time()) ||
                        (initializerScript && initializerScript->status().modifiedAt > time->time());
            }
        }

        if(!needParse) return;

        // The package identifier and path are automatically set.
        metadata.set("id", identifierForFile(packageFile));
        metadata.set("path", packageFile.path());

        // Check for a ScriptedInfo source.
        if(metadataInfo)
        {
            ScriptedInfo script(&metadata);
            script.parse(*metadataInfo);

            parsedAt = metadataInfo->status().modifiedAt;
        }

        // Check for an initialization script.
        if(initializerScript)
        {
            Script script(*initializerScript);
            Process proc(&metadata);
            proc.run(script);
            proc.execute();

            if(parsedAt.isValid() && initializerScript->status().modifiedAt > parsedAt)
            {
                parsedAt = initializerScript->status().modifiedAt;
            }
        }

        metadata.addTime(TIMESTAMP, parsedAt);

        LOG_RES_MSG("Parsed metadata of '%s':\n")
                << identifierForFile(packageFile)
                << packageFile.info().asText();
    }
}

void Package::validateMetadata(Record const &packageInfo)
{
    char const *required[] = {
        "title", "version", "license", "tags", 0
    };

    for(int i = 0; required[i]; ++i)
    {
        if(!packageInfo.has(required[i]))
        {
            throw IncompleteMetadataError("Package::validateMetadata",
                                          QString("Package \"%1\" does not have '%2' in its metadata")
                                          .arg(packageInfo.gets("path"))
                                          .arg(required[i]));
        }
    }
}

static String stripAfterFirstUnderscore(String str)
{
    int pos = str.indexOf('_');
    if(pos > 0) return str.left(pos);
    return str;
}

static String extractIdentifier(String str)
{
    return stripAfterFirstUnderscore(str.fileNameWithoutExtension());
}

String Package::identifierForFile(File const &file)
{
    // Form the prefix if there are enclosing packs as parents.
    String prefix;
    Folder const *parent = file.parent();
    while(parent && parent->name().fileNameExtension() == ".pack")
    {
        prefix = extractIdentifier(parent->name()) + "." + prefix;
        parent = parent->parent();
    }
    return prefix + extractIdentifier(file.name());
}

} // namespace de
