/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.track;

import de.janquadflieg.mrracer.classification.Situations;
import de.janquadflieg.mrracer.Utils;
import java.io.*;
import java.util.*;
import java.util.zip.*;

/**
 *
 * @author Jan Quadflieg
 */
public class TrackDB
        implements Serializable {

    public static final TrackModel UNKNOWN_MODEL = null;
    static final long serialVersionUID = 543417241306257651L;
    private ArrayList<TrackModel> tracks = new ArrayList<TrackModel>();
    private static final double EPSILON = 0.5;
    //private HashMap<Integer, HashMap<Integer, ArrayList<TrackModel>>> fastLookupMap =
    //         new HashMap<Integer, HashMap<Integer, ArrayList<TrackModel>>>();

    public TrackDB() {
    }

    public void add(TrackModel tm) {
        tracks.add(tm);

//        HashMap<Integer, ArrayList<TrackModel>> map;
//        Integer widthKey = new Integer((int) Math.round(tm.getWidth()));
//
//        if (fastLookupMap.containsKey(widthKey)) {
//            map = fastLookupMap.get(widthKey);
//
//        } else {
//            map = new HashMap<Integer, ArrayList<TrackModel>>();
//            fastLookupMap.put(widthKey, map);
//        }
//
//        for (int i = 1; i <= tm.gridPositions(); ++i) {
//            ArrayList<TrackModel> list;
//            Integer posKey = new Integer(i);
//
//            if (map.containsKey(posKey)) {
//                list = map.get(posKey);
//
//            } else {
//                list = new ArrayList<TrackModel>();
//                map.put(posKey, list);
//            }
//
//            list.add(tm);
//            Collections.sort(list, new TrackDB.TrackModelComparator(i));
//        }
    }

//    public boolean contains() {
//        return false;
//    }
//
//    public TrackModel get(double length) {
//        return null;
//    }
    public TrackModel get(int position, double distance, double width) {
        ArrayList<TrackModel> results = new ArrayList<TrackModel>();

        for (TrackModel tm : tracks) {
            //tm.print();
            //System.out.println(tm.getName()+" "+position+" "+tm.getGridPosition(position));
            double d = tm.getGridPosition(position);
            double w = tm.getWidth();

            //System.out.println((distance-EPSILON)+" "+d+" "+(distance+EPSILON));

            if (distance - EPSILON < d && d < distance + EPSILON &&
                    width - EPSILON < w && w < width + EPSILON) {
                //return tm;
                results.add(tm);
            }
        }

        if (results.isEmpty()) {
            return null;

        } else {
            return results.get(0);

        } /*else {
    System.out.println("Warning, found "+results.size()+" possible results for position "+
    position+" at "+distance);
    for(TrackModel tm: results){
    System.out.println(tm.getName());
    }
    System.exit(0);
    return null;
    }*/
    }

    public TrackModel getByName(String name){
        ArrayList<TrackModel> results = new ArrayList<TrackModel>();

        for (TrackModel tm : tracks) {
            if (tm.getName().equalsIgnoreCase(name)) {
                //return tm;
                results.add(tm);
            }
        }

        if (results.isEmpty()) {
            return UNKNOWN_MODEL;

        } else {
            return results.get(0);
        }
    }

//    public TrackModel fastGet(int position, double distance, double width){
//        Integer widthKey = new Integer((int)Math.round(width));
//
//        if(fastLookupMap.containsKey(widthKey)){
//            HashMap<Integer, ArrayList<TrackModel>> map = fastLookupMap.get(widthKey);
//            Integer positionKey = new Integer(position);
//
//            if(map.containsKey(positionKey)){
//                ArrayList<TrackModel> list = map.get(positionKey);
//                TrackModel dummy = new TrackModel();
//                dummy.setGridPosition(position, distance);
//                int index = Collections.binarySearch(list,
//                        dummy, new TrackDB.TrackModelComparator(position));
//
//                if(index >= 0){
//                    return list.get(index);
//                }
//            }
//        }
//
//        return UNKNOWN_MODEL;
//    }

    public static TrackDB create() {
        try {
            InputStream in = new Object().getClass().getResourceAsStream("/de/janquadflieg/mrracer/data/trackdb");
            ZipInputStream zin = new ZipInputStream(in);
            ZipEntry entry = zin.getNextEntry();
            ObjectInputStream oin = new ObjectInputStream(zin);
            TrackDB result = (TrackDB) oin.readObject();

            oin.close();

            TrackDB.loadTracks(result);

            return result;

        } catch (Exception e) {
            e.printStackTrace(System.out);

            return new TrackDB();
        }
    }

    private static void loadTracks(TrackDB trackDB) {
        java.io.File currentDirectory = new java.io.File(".");
        java.io.File[] files = currentDirectory.listFiles();
        //System.out.println("Looking for trackmodels in dir " + currentDirectory.getAbsolutePath());

        for (java.io.File f : files) {
            if (f.isFile() && f.getName().endsWith(TrackModel.TM_EXT)) {
                try {
                    String filename = "." + java.io.File.separator + f.getName();
                    //System.out.println("Loading trackmodel from file \"" + filename + "\".");
                    trackDB.add(TrackModel.load(filename));

                } catch (Exception e) {
                    e.printStackTrace(System.out);
                }
            }
        }
    }

    public int size(){
        return this.tracks.size();
    }

    public TrackModel get(int i){
        return tracks.get(i);
    }

    public static void main(String[] args) {
        try {
//            TrackDB loaded = TrackDB.create();
//            for (TrackModel t : loaded.tracks) {
//                t.print();
//                System.out.println("");
//                System.out.println(t.getName());
//                System.out.println("Apexes:");
//                for(int i=0; i < t.size(); ++i){
//                    TrackSegment ts = t.getSegment(i);
//                    if(ts.isCorner()){
//                        System.out.println(ts.toString());
//                        double[] a = ts.getApexes();
//                        for(int j=0; j < a.length; ++j){
//                            String s = "["+j+"] "+Utils.dTS(a[j])+" / ";
//                            s += Utils.dTS(a[j]-ts.getStart()) + " ";
//                            if(ts.getType() != Situations.TYPE_FULL && j==0){
//                                s += Utils.dTS(ts.getSubSegment(0).getLength()/t.getWidth())+" ";
//                            }
//                            s += Situations.toShortString(ts.getSubSegment(ts.getIndex(a[j])).getType())+" ";
//                            System.out.print(s);
//                        }
//                        System.out.println("");
//                    }
//                }
//                System.out.println("");
//            }
            if (1 != 1) {
                return;
            }



            // code, um die datenbank zu erzeugen
            TrackDB db = new TrackDB();

            TrackModel tm;
            //File directory = new File("/home/quad/Diplomarbeit/Data/Tracks/NewTrackModels");
            //File directory = new File("g:\\Studium\\Diplomarbeit\\Data\\Tracks\\TrackModels");
            File directory = new File("f:\\quad\\svn\\Diplomarbeit\\Data\\Tracks\\NewTrackModels");
            
            File[] files = directory.listFiles();

            System.out.println(directory+" "+files.length+" files");

            java.util.Arrays.sort(files);

            for (int i = 0; i < files.length; ++i) {
                if (!(files[i].isDirectory() ||
                        files[i].getAbsolutePath().contains(".svn") ||
                        files[i].getAbsolutePath().endsWith("~"))) {
                    System.out.println(files[i].getAbsolutePath());
                    tm = TrackModel.load(files[i].getAbsolutePath());
                    db.add(tm);
                //System.out.println("");
                    tm.print();
                }
            }

//            Iterator<Integer> it = db.fastLookupMap.keySet().iterator();
//            while(it.hasNext()){
//                //System.out.println("***********************************");
//                Integer width = it.next();
//                //System.out.println("Width: "+width);
//                HashMap<Integer, ArrayList<TrackModel>> map = db.fastLookupMap.get(width);
//
//                Iterator<Integer> it2 = map.keySet().iterator();
//                while(it2.hasNext()){
//                    Integer position = it2.next();
//                    //System.out.println("Position: "+position);
//                    ArrayList<TrackModel> list = map.get(position);
//                    Iterator<TrackModel> it3 = list.iterator();
//                    while(it3.hasNext()){
//                        TrackModel model = it3.next();
//                        //System.out.println(model.getName()+", "+model.getGridPosition(position));
//                    }
//                }
//            }

            boolean zip = true;
            //String fileName = "/home/quad/Diplomarbeit/Code/projects/MrRacer/src/de/janquadflieg/mrracer/data/trackdb";
            //String fileName = "g:\\Studium\\Diplomarbeit\\Code\\projects\\MrRacer\\src\\de\\janquadflieg\\mrracer\\data\\trackdb";
            String fileName = "f:\\quad\\svn\\Diplomarbeit\\Code\\projects\\MrRacer\\src\\de\\janquadflieg\\mrracer\\data\\trackdb";

            FileOutputStream fo = new FileOutputStream(fileName);
            ObjectOutputStream oo = null;
            ZipOutputStream zo = null;

            if (zip) {
                zo = new ZipOutputStream(fo);

                ZipEntry entry = new ZipEntry("trackdb");
                zo.putNextEntry(entry);
                zo.setLevel(9);

                oo = new ObjectOutputStream(zo);

            } else {
                oo = new ObjectOutputStream(fo);
            }

            oo.writeObject(db);
            oo.flush();

            if (zip) {
                zo.closeEntry();
            }

            oo.close();
            fo.close();

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static class TrackModelComparator
            implements Comparator<TrackModel>{
        private int position;

        public TrackModelComparator(int i){
            position = i;
        }

        @Override
        public int compare(TrackModel tm1, TrackModel tm2){
            if(Math.abs(tm1.getGridPosition(position)-tm2.getGridPosition(position)) < EPSILON){
                return 0;

            } else if (tm1.getGridPosition(position) < tm2.getGridPosition(position)){
                return -1;

            } else {
                return 1;
            }
        }
    }
}
/*
public class JarReader extends Thread{

File jarFile; // the original jar file
File tempFile;// the temp file for the jar entries
File temp;//the temp file for the names vector
Vector outputNames;// contains the strings from a file
JarOutputStream jos;// output to temp jar file
ZipEntry ze;

public JarReader(String fName, Vector v) {
jarFile = new File(fName);
outputNames = v;
}

public void run(){
try{
// write the names to a temp file
temp = File.createTempFile("names", null);
BufferedWriter out = new BufferedWriter(new FileWriter(temp));
Enumeration enum = outputNames.elements();
while(enum.hasMoreElements()){
out.write(enum.nextElement().toString());
out.newLine();
}
out.close();
}
catch(Exception fe){
fe.printStackTrace();
}

// write the jar files
try{
JarFile jf = new JarFile(jarFile);
tempFile = new File("temp.tmp");
jos = new JarOutputStream(new FileOutputStream(tempFile));
for(Enumeration enum = jf.entries(); enum.hasMoreElements(); ){
ze = new ZipEntry((ZipEntry)enum.nextElement());
if(!(ze.getName().equals("names.dat"))){// exclude names.dat
readWrite(jf.getInputStream(ze));
}
}
ZipEntry ze = new ZipEntry("names.dat");
readWrite(new FileInputStream(temp));
jos.close();
jf.close();// needed to allow correct renaming on exit

// replace the current jar file with the new file at JVM exit
Runtime r = Runtime.getRuntime();r.addShutdownHook(new Thread(){
public void run() {
jarFile.delete();
tempFile.renameTo(jarFile);
}
});


}
catch(Exception e){
e.printStackTrace();
}
}

// the method to read the jar file and write the bytes to the temp file
void readWrite(InputStream in){
try{
jos.putNextEntry(ze);
int i;
while((i = in.read()) != -1){
jos.write(i);
jos.flush();
}
in.close();
}
catch(IOException ie){
ie.printStackTrace();
}
}
}*/




