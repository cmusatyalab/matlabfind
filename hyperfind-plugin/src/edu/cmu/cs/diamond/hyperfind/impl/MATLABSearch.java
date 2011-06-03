/*
 *  MATLAB plugin, a search plugin for the OpenDiamond platform
 *
 *  Copyright (c) 2009-2011 Carnegie Mellon University
 *  All rights reserved.
 *
 *  MATLAB plugin is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2.
 *
 *  MATLAB plugin is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with MATLAB plugin. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Linking MATLAB plugin statically or dynamically with other modules is
 *  making a combined work based on MATLAB plugin. Thus, the terms and
 *  conditions of the GNU General Public License cover the whole
 *  combination.
 *
 *  In addition, as a special exception, the copyright holders of
 *  MATLAB plugin give you permission to combine MATLAB plugin with free
 *  software programs or libraries that are released under the GNU LGPL or the
 *  Eclipse Public License 1.0. You may copy and distribute such a system
 *  following the terms of the GNU GPL for MATLAB plugin and the licenses of
 *  the other code concerned, provided that you include the source code of
 *  that other code when and as the GNU GPL requires distribution of source
 *  code.
 *
 *  Note that people who make modified versions of MATLAB plugin are not
 *  obligated to grant this special exception for their modified versions;
 *  it is their choice whether to do so. The GNU General Public License
 *  gives permission to release a modified version without this exception;
 *  this exception also makes it possible to release a modified version
 *  which carries forward this exception.
 */

package edu.cmu.cs.diamond.hyperfind.impl;

import java.awt.Component;
import java.awt.image.BufferedImage;
import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import edu.cmu.cs.diamond.hyperfind.BoundingBox;
import edu.cmu.cs.diamond.hyperfind.HyperFindSearch;
import edu.cmu.cs.diamond.hyperfind.SearchSettingsFrame;
import edu.cmu.cs.diamond.opendiamond.*;

public class MATLABSearch extends HyperFindSearch {

    private final byte[] blob;

    private final String searchName;

    private final String initMacroName;

    private final String evalMacroName;

    private final String digestedName;

    private final SearchSettingsFrame settings;

    MATLABSearch(String searchName, String initMacroName,
            String evalMacroName, byte[] blob) {
        this.searchName = searchName;
        this.initMacroName = initMacroName;
        this.evalMacroName = evalMacroName;
        this.blob = blob;
        digestedName = digest(searchName.getBytes(), initMacroName.getBytes(),
                evalMacroName.getBytes(), blob);

        this.settings = new SearchSettingsFrame(searchName, "filter", true,
                1, true);
        final MATLABSearch search = this;
        settings.addChangeListener(new ChangeListener() {
            @Override
            public void stateChanged(ChangeEvent e) {
                search.fireChangeEvent();
            }
        });
    }

    public static byte[] createBlob(Map<String, byte[]> files) {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        ZipOutputStream zos = new ZipOutputStream(baos);
        try {
            for (Map.Entry<String, byte[]> entry: files.entrySet()) {
                ZipEntry ze = new ZipEntry(entry.getKey());
                // storing different timestamps on every run would defeat
                // server-side result caching
                ze.setTime(0);
                zos.putNextEntry(ze);
                zos.write(entry.getValue(), 0, entry.getValue().length);
            }
            zos.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return baos.toByteArray();
    }

    @Override
    public void addPatches(List<BufferedImage> patches) throws IOException,
            InterruptedException {
    }

    @Override
    public void edit(Component parentComponent) throws IOException,
            InterruptedException {
        settings.setVisible(true);
    }

    @Override
    public String getInstanceName() {
        return settings.getInstanceName();
    }

    @Override
    public String getDigestedName() {
        return digestedName;
    }

    @Override
    public String getSearchName() {
        return searchName;
    }

    @Override
    public boolean isEditable() {
        return true;
    }

    @Override
    public boolean needsPatches() {
        return false;
    }

    @Override
    public List<BoundingBox> runLocally(BufferedImage image)
            throws IOException, InterruptedException {
        List<BoundingBox> empty = Collections.emptyList();
        return empty;
    }

    @Override
    public List<Filter> createFilters() throws IOException {
        FileInputStream in = null;
        FilterCode c = null;
        try {
            in = new FileInputStream("/opt/snapfind/lib/fil_matlab_exec");
            c = new FilterCode(in);
        } finally {
            if (in != null) {
                try {
                    in.close();
                } catch (IOException ignore) {
                }
            }
        }
        List<String> dependencies = new ArrayList<String>();
        dependencies.add("RGB");
        List<String> args = new ArrayList<String>();
        args.add(initMacroName);
        args.add(evalMacroName);
        Filter f = new Filter(getDigestedName(), c, settings.getThreshold(),
                dependencies, args, blob);

        List<Filter> filters = new ArrayList<Filter>();
        filters.add(f);
        return filters;
    }

    @Override
    public void dispose() {
        settings.dispose();
    }
}
