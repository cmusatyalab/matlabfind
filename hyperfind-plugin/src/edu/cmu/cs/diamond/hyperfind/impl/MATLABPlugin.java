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
 *  software programs or libraries that are released under the GNU LGPL or
 *  the Eclipse Public License 1.0. You may copy and distribute such a system
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

import java.awt.image.BufferedImage;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import edu.cmu.cs.diamond.hyperfind.HyperFindSearch;
import edu.cmu.cs.diamond.hyperfind.HyperFindSearchFactory;
import edu.cmu.cs.diamond.hyperfind.HyperFindSearchType;

public class MATLABPlugin extends HyperFindSearchFactory {

    @Override
    public HyperFindSearch createHyperFindSearch() {
        return null;
    }

    @Override
    public HyperFindSearch createHyperFindSearch(List<BufferedImage> patches) {
        return createHyperFindSearch();
    }

    @Override
    public HyperFindSearch createHyperFindSearchFromZipMap(
            Map<String, byte[]> zipMap, Properties p) {
        String sig = p.getProperty("Plugin");
        if (sig == null || !sig.trim().equals("MATLAB")) {
            return null;
        }

        return new MATLABSearch(p.getProperty("Name"),
                p.getProperty("InitFunction"), p.getProperty("EvalFunction"),
                MATLABSearch.createBlob(zipMap));
    }

    @Override
    public String getDisplayName() {
        return "MATLAB";
    }

    @Override
    public HyperFindSearchType getType() {
        return HyperFindSearchType.FILTER;
    }

    @Override
    public boolean needsPatches() {
        return false;
    }

    @Override
    public boolean needsBundle() {
        return true;
    }
}
