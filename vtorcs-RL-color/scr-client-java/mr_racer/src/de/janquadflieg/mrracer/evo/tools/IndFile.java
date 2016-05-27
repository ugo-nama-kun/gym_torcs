/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package de.janquadflieg.mrracer.evo.tools;

import java.util.ArrayList;

/**
 *
 * @author quad
 */
public class IndFile {
    public String file;
    public String track;
    public ArrayList<ArrayList<String>> entries = new ArrayList<ArrayList<String>>();
    public ArrayList<String> header = new ArrayList<String>();

    public ArrayList<String> getLine(int indno){
        ArrayList<String> result = null;

        for(int i=0; i < entries.size() && result == null; ++i){
            if(Integer.parseInt(entries.get(i).get(0)) == indno){
                result = entries.get(i);
            }
        }

        return result;
    }

    public String getEntry(ArrayList<String> line, String s){
        return line.get(header.indexOf(s));
    }
}