/* Double Contact
 *
 * Module: console convertor application class
 *
 * Copyright 2016 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include "QFile"
#include "QFileInfo"
#include <QStringList>
#include "convertor.h"
#include "formats/formatfactory.h"
#include "formats/files/mpbfile.h"
#include "formats/files/udxfile.h"
#include "formats/files/vcfdirectory.h"
#include "formats/files/vcffile.h"

Convertor::Convertor(int &argc, char **argv)
    : QCoreApplication(argc, argv),
      out(stdout)
{
}

// Main program loop
int Convertor::start()
{
    out << tr("Contact convertor by Mikhail Y. Zvyozdochkin\n");
    // Parse command line
    if (arguments().count()<2) {
        printUsage();
        return 1;
    }
    QString inPath, outPath, outFormat;
    bool infoMode = false;
    bool forceOverwrite = false;
    bool forceSingleFile = false;
    bool forceDirectory = false;
    bool swapNames = false;
    bool splitNames = false;
    bool generateFullNames = false;
    bool dropFullNames = false;
    bool reverseFullNames = false;
    bool dropSlashes = false;
    for (int i=1; i<arguments().count(); i++) {
        if (arguments()[i]=="-i" || arguments()[i]=="--info") {
            i++;
            if (i==arguments().count()) {
                out << tr("Error: %1 option present, but file path is missing\n").arg(arguments()[i-1]);
                printUsage();
                return 2;
            }
            inPath = arguments()[i];
            if (arguments()[i-1]=="--info")
                    infoMode = true;
            continue;
        }
        else if (arguments()[i]=="-o") {
            i++;
            if (i==arguments().count()) {
                out << tr("Error: -o option present, but file path is missing\n");
                printUsage();
                return 3;
            }
            outPath = arguments()[i];
            continue;
        }
        else if (arguments()[i]=="-f") {
            i++;
            if (i==arguments().count()) {
                out << tr("Error: -f option present, but format name is missing\n");
                printUsage();
                return 4;
            }
            outFormat = arguments()[i];
            if (outFormat!="vcf21" && outFormat!="vcf30" && outFormat!="vcfauto" && outFormat!="udx" && outFormat!="mpb" && outFormat!="copy") {
                out << tr("Error: Unknown output format: %1\n").arg(outFormat);
                printUsage();
                return 5;
            }
            continue;
        }
        else if (arguments()[i]=="-w")
            forceOverwrite = true;
        else if (arguments()[i]=="-s")
            forceSingleFile = true;
        else if (arguments()[i]=="-d")
            forceDirectory = true;
        else if (arguments()[i]=="--swap-names")
            swapNames = true;
        else if (arguments()[i]=="--split-names")
            splitNames = true;
        else if (arguments()[i]=="--generate-full-names")
            generateFullNames = true;
        else if (arguments()[i]=="--drop-full-names")
            dropFullNames = true;
        else if (arguments()[i]=="--reverse-full-names")
            reverseFullNames = true;
        else if (arguments()[i]=="--drop-slashes")
            dropSlashes = true;
        else {
            out << tr("Unknown option: %1\n").arg(arguments()[i]);
            printUsage();
            return 6;
        }
    }
    // Check input data completion
    if (inPath.isEmpty()) {
        out << tr("Error: Input path is missing\n");
        printUsage();
        return 7;
    }
    if (outPath.isEmpty() && !infoMode) {
        out << tr("Error: Output path is missing\n");
        printUsage();
        return 8;
    }
    if (outFormat.isEmpty() && !infoMode) {
        out << tr("Error: Output format name is missing\n");
        printUsage();
        return 9;
    }
    if (forceSingleFile && forceDirectory) {
        out << tr("Error: Options -s and -d are not compatible\n");
        printUsage();
        return 10;
    }
    if (forceDirectory && !outFormat.contains("vcf") && outFormat!="copy") {
        out << tr("Error: -d option applicable only for vCard format");
        return 11;
    }
    if (infoMode && !(outPath.isEmpty() && outFormat.isEmpty())) {
        out << tr("Error: Command --info is not compatible with -o and -f options\n");
        printUsage();
        return 12;
    }
    // Check if output file exists
    QFile of(outPath);
    if (of.exists() && !forceOverwrite && !QFileInfo(outPath).isDir()) {
        out << tr("Error: Output file already exists, use -w if necessary\n");
        printUsage();
        return 13;
    }
    // Define, create file or directory at output
    // (default: as input)
    FormatType ift = QFileInfo(inPath).isDir() ? ftDirectory : ftFile;
    FormatType oft = ift;
    if (forceSingleFile)
        oft = ftFile;
    else if (forceDirectory)
        oft = ftDirectory;
    // Read
    IFormat* iFormat = 0;
    FormatFactory factory;
    if (ift==ftFile)
        iFormat = factory.createObject(inPath);
    else
        iFormat = new VCFDirectory();
    if (!iFormat) {
        out << factory.error << "\n";
        return 14;
    }
    ContactList items;
    bool res = iFormat->importRecords(inPath, items, false);
    logFormat(iFormat);
    delete iFormat;
    if (!res)
        return 15;
    out << tr("%1 records read\n").arg(items.count());
    // Show statistics, if info mode switched on
    if (infoMode) {
        out << "\n" << items.statistics() << "\n";
        return 0;
    }
    // Conversions
    for (int i=0; i<items.count(); i++) {
        if (swapNames)
            items[i].swapNames();
        if (splitNames)
            items[i].splitNames();
        // TODO splitNumbers now can't be implemented, because in GUI it works via ContactModel
        // Probably, move it in ContactList in future
        if (generateFullNames)
            items[i].fullName = items[i].formatNames();
        if (dropFullNames)
            items[i].fullName.clear();
        if (reverseFullNames)
            items[i].reverseFullName();
        if (dropSlashes)
            items[i].dropSlashes();
        // TODO intlPhonePrefix implement after CountryManager create
        // items[i].intlPhonePrefix(cRule);
    }
    //Define output format
    gd.preferredVCFVersion = GlobalConfig::VCF21;
    IFormat* oFormat = 0;
    if (oft==ftDirectory)
        oFormat = new VCFDirectory();
    else if (outFormat.contains("vcf")) {
        if (outFormat=="vcf30")
            gd.preferredVCFVersion = GlobalConfig::VCF30;
        gd.useOriginalFileVersion = (outFormat=="vcfauto");
        oFormat = new VCFFile();
    }
    else if (outFormat.contains("udx"))
        oFormat = new UDXFile();
    else if (outFormat.contains("mpb"))
        oFormat = new MPBFile();
    else { // copy input format
        if (VCFFile::detect(inPath)) {
            gd.useOriginalFileVersion = true;
            oFormat = new VCFFile();
        }
        else if (UDXFile::detect(inPath))
            oFormat = new UDXFile();
        else if (MPBFile::detect(inPath))
            oFormat = new MPBFile();
        else {
            out << "Error: Can't autodetect input format\n";
            return 16;
        }
    }
    // Write
    res = oFormat->exportRecords(outPath, items);
    logFormat(oFormat);
    delete oFormat;
    out << "Output file successfully written\n";
    return res ? 0 : 17;
}

// Print program usage
void Convertor::printUsage()
{
    out << tr(
        "Usage:\n" \
        "contconv -i inputfile -o outfile -f outformat [-w] [-d|-s] [commands]\n" \
        "contconv --info inputfile\n" \
        "\n" \
        "Possible values for outformat:\n" \
        "copy - same as input format, if atodetected\n" \
        "vcf21 - vCard version 2.1\n" \
        "vcf30 - vCard version 3.0\n" \
        "vcfauto - vCard version as in input file\n" \
        "udx - Philips Xenium UDX\n" \
        "mpb - MyPhoneExplorer backup\n" \
        "\n" \
        "Options:\n" \
        "-w - force overwrite output single file, if exists (directories overwrites already)\n" \
        "-s - write VCF as single file (by default, write as in input)\n" \
        "-d - write VCFs as directory (not compatible with -d)\n" \
        "Commands:\n" \
        "--swap-names - swap first and last name" \
        "--split-names - split name by spaces" \
        "--generate-full-names - generate full (formatted) name by names" \
        "--drop-full-names - clear full (formatted) name" \
        "--reverse-full-names - swap parts of full (formatted) name"
        "--drop-slashes - remove back slashes and other SIM-legacy from names" \
        "--info - show statistic info about inputfile (incompatible with -o and -f options)" \
        "\n"); // TODO filter
}

void Convertor::logFormat(IFormat* format)
{
    foreach (const QString& s, format->errors())
        out << s << "\n";
    if (!format->fatalError().isEmpty())
        out << "Error: " << format->fatalError() << "\n";
}
