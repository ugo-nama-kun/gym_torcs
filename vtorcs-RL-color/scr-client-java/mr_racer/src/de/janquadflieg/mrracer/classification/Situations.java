/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.classification;

import java.util.*;


/**
 *
 * @author Jan Quadflieg
 */
public class Situations {
    // Fun with bitmasks
    // bit
    // 31 ... 24     |    23 ... 16   |     15 ... 0
    // error codes   |  directions    |      type

    /** Constant indicating that there is no error. */
    public static final int ERROR_NONE = (0x1 << 24);
    ///** Constant indicating that the classifier was unable to classify. */
    public static final int ERROR_UNABLE_TO_CLASSIFY = (0x2 << 24);
    /** Mask for the error. */
    public static final int ERROR_MASK = (0xFF << 24);
    /** Constant indicating forward. */
    public static final int DIRECTION_FORWARD = (0x1 << 16);
    /** Constant indicating left. */
    public static final int DIRECTION_LEFT = (0x2 << 16);
    /** Constant indicating right. */
    public static final int DIRECTION_RIGHT = (0x4 << 16);
    /** Mask for the direction. */
    public static final int DIRECTION_MASK = (0xFF << 16);
    /** Constant indicating that the car is outside the track. */
    public static final int TYPE_OUTSIDE = 0x2;
    /** Constant indicating a hairpin. */
    public static final int TYPE_HAIRPIN = 0x8;
    /** Constant indicating a corner which can be driven at slow speed. */
    public static final int TYPE_SLOW = 0x20;
    /** Constant indicating a corner which can be driven at medium speed. */
    public static final int TYPE_MEDIUM = 0x80;
    /** Constant indicating a corner which can be driven at full speed. */
    public static final int TYPE_FULL = 0x200;
    /** Constant indicating a straight with a corner in sight. */
    public static final int TYPE_STRAIGHT_AC = 0x800;
    /** Constant indicating a straight. */
    public static final int TYPE_STRAIGHT = 0x2000;

    public static final int[] FIVE_TYPES ={TYPE_OUTSIDE,
        TYPE_HAIRPIN, TYPE_SLOW, TYPE_MEDIUM, TYPE_FULL,
        TYPE_STRAIGHT};

    public static final int[] ALL_TYPES = {Situations.TYPE_STRAIGHT,
        Situations.TYPE_STRAIGHT_AC, Situations.TYPE_FULL, Situations.TYPE_MEDIUM,
        Situations.TYPE_SLOW, Situations.TYPE_HAIRPIN};

    /** Mask for the type. */
    public static final int TYPE_MASK = 0xFFFF;

    /** Constant indicating a straight. */
    public static final int STRAIGHT = ERROR_NONE | DIRECTION_FORWARD | TYPE_STRAIGHT;
    /** Constant indicating a straight, approaching a corner.*/
    public static final int STRAIGHT_AC = ERROR_NONE | DIRECTION_FORWARD | TYPE_STRAIGHT_AC;
    /** Right hand corner which can be driven at full speed. */
    public static final int FULL_RIGHT = ERROR_NONE | DIRECTION_RIGHT | TYPE_FULL;
    /** Right hand corner which can be driven at medium speed. */
    public static final int MEDIUM_RIGHT = ERROR_NONE | DIRECTION_RIGHT | TYPE_MEDIUM;
    /** Right hand corner which can be driven at slow speed. */
    public static final int SLOW_RIGHT = ERROR_NONE | DIRECTION_RIGHT | TYPE_SLOW;
    /** Right hairpin. */
    public static final int HAIRPIN_RIGHT = ERROR_NONE | DIRECTION_RIGHT | TYPE_HAIRPIN;
    /** Left hand corner which can be driven at full speed. */
    public static final int FULL_LEFT = ERROR_NONE | DIRECTION_LEFT | TYPE_FULL;
    /** Left hand corner which can be driven at medium speed. */
    public static final int MEDIUM_LEFT = ERROR_NONE | DIRECTION_LEFT | TYPE_MEDIUM;
    /** Left hand corner which can be driven at slow speed. */
    public static final int SLOW_LEFT = ERROR_NONE | DIRECTION_LEFT | TYPE_SLOW;
    /** Right hairpin. */
    public static final int HAIRPIN_LEFT = ERROR_NONE | DIRECTION_LEFT | TYPE_HAIRPIN;
    
    /** Map from Strings to ints. */
    private static HashMap<String, Integer> STRING_TO_INTS = new HashMap<String, Integer>();    

    /** Map from ints to Strings. */
    private static HashMap<Integer, String> INTS_TO_STRINGS = new HashMap<Integer, String>();

    /** Map from ints to short Strings. */
    private static HashMap<Integer, String> INTS_TO_SHORT_STRINGS = new HashMap<Integer, String>();

    /** Map from ints to ints, to mirror situations. */
    private static HashMap<Integer, Integer> MIRROR = new HashMap<Integer, Integer>();

    /** List containing the complete situations on track as strings. */
    private static ArrayList<String> SITUATIONS_AS_STRINGS = new ArrayList<String>();
    

    static{
        // error codes
        STRING_TO_INTS.put("ERROR_NONE", ERROR_NONE);
        STRING_TO_INTS.put("ERROR_UNABLE_TO_CLASSIFY", ERROR_UNABLE_TO_CLASSIFY);
        // directions
        STRING_TO_INTS.put("DIRECTION_FORWARD", DIRECTION_FORWARD);
        STRING_TO_INTS.put("DIRECTION_LEFT", DIRECTION_LEFT);
        STRING_TO_INTS.put("DIRECTION_RIGHT", DIRECTION_RIGHT);
        // types
        STRING_TO_INTS.put("TYPE_OUTSIDE", TYPE_OUTSIDE);
        STRING_TO_INTS.put("TYPE_HAIRPIN", TYPE_HAIRPIN);
        STRING_TO_INTS.put("TYPE_SLOW", TYPE_SLOW);
        STRING_TO_INTS.put("TYPE_MEDIUM", TYPE_MEDIUM);
        STRING_TO_INTS.put("TYPE_FULL", TYPE_FULL);
        STRING_TO_INTS.put("TYPE_STRAIGHT_AC", TYPE_STRAIGHT_AC);
        STRING_TO_INTS.put("TYPE_STRAIGHT", TYPE_STRAIGHT);
        // complete situations when on the track
        STRING_TO_INTS.put("STRAIGHT", STRAIGHT);
        STRING_TO_INTS.put("STRAIGHT_AC", STRAIGHT_AC);        
        STRING_TO_INTS.put("FULL_RIGHT", FULL_RIGHT);
        STRING_TO_INTS.put("FULL_LEFT", FULL_LEFT);
        STRING_TO_INTS.put("MEDIUM_RIGHT", MEDIUM_RIGHT);
        STRING_TO_INTS.put("MEDIUM_LEFT", MEDIUM_LEFT);
        STRING_TO_INTS.put("SLOW_RIGHT", SLOW_RIGHT);
        STRING_TO_INTS.put("SLOW_LEFT", SLOW_LEFT);
        STRING_TO_INTS.put("HAIRPIN_LEFT", HAIRPIN_LEFT);
        STRING_TO_INTS.put("HAIRPIN_RIGHT", HAIRPIN_RIGHT);
        //STRING_TO_INTS.put("", );

        // the same a mapping int -> string
        // error codes
        INTS_TO_STRINGS.put(ERROR_NONE, "ERROR_NONE");
        INTS_TO_STRINGS.put(ERROR_UNABLE_TO_CLASSIFY, "ERROR_UNABLE_TO_CLASSIFY");
        // directions
        INTS_TO_STRINGS.put(DIRECTION_FORWARD, "DIRECTION_FORWARD");
        INTS_TO_STRINGS.put(DIRECTION_LEFT, "DIRECTION_LEFT");
        INTS_TO_STRINGS.put(DIRECTION_RIGHT, "DIRECTION_RIGHT");
        // types
        INTS_TO_STRINGS.put(TYPE_OUTSIDE, "TYPE_OUTSIDE");
        INTS_TO_STRINGS.put(TYPE_HAIRPIN, "TYPE_HAIRPIN");
        INTS_TO_STRINGS.put(TYPE_SLOW, "TYPE_SLOW");
        INTS_TO_STRINGS.put(TYPE_MEDIUM, "TYPE_MEDIUM");
        INTS_TO_STRINGS.put(TYPE_FULL, "TYPE_FULL");
        INTS_TO_STRINGS.put(TYPE_STRAIGHT_AC, "TYPE_STRAIGHT_AC");
        INTS_TO_STRINGS.put(TYPE_STRAIGHT, "TYPE_STRAIGHT");
        // complete situations when on the track
        INTS_TO_STRINGS.put(STRAIGHT, "STRAIGHT");
        INTS_TO_STRINGS.put(STRAIGHT_AC, "STRAIGHT_AC");
        INTS_TO_STRINGS.put(FULL_RIGHT, "FULL_RIGHT");
        INTS_TO_STRINGS.put(FULL_LEFT, "FULL_LEFT");
        INTS_TO_STRINGS.put(MEDIUM_RIGHT, "MEDIUM_RIGHT");
        INTS_TO_STRINGS.put(MEDIUM_LEFT, "MEDIUM_LEFT");
        INTS_TO_STRINGS.put(SLOW_RIGHT, "SLOW_RIGHT");
        INTS_TO_STRINGS.put(SLOW_LEFT, "SLOW_LEFT");
        INTS_TO_STRINGS.put(HAIRPIN_LEFT, "HAIRPIN_LEFT");
        INTS_TO_STRINGS.put(HAIRPIN_RIGHT, "HAIRPIN_RIGHT");

        // the same a mapping int -> short string, for compact debug output
        // error codes
        INTS_TO_SHORT_STRINGS.put(ERROR_NONE, "E_N");
        INTS_TO_SHORT_STRINGS.put(ERROR_UNABLE_TO_CLASSIFY, "E_U");
        // directions
        INTS_TO_SHORT_STRINGS.put(DIRECTION_FORWARD, "D_F");
        INTS_TO_SHORT_STRINGS.put(DIRECTION_LEFT, "D_L");
        INTS_TO_SHORT_STRINGS.put(DIRECTION_RIGHT, "D_R");
        // types
        INTS_TO_SHORT_STRINGS.put(TYPE_OUTSIDE, "T_O");
        INTS_TO_SHORT_STRINGS.put(TYPE_HAIRPIN, "T_H");
        INTS_TO_SHORT_STRINGS.put(TYPE_SLOW, "T_S");
        INTS_TO_SHORT_STRINGS.put(TYPE_MEDIUM, "T_M");
        INTS_TO_SHORT_STRINGS.put(TYPE_FULL, "T_F");
        INTS_TO_SHORT_STRINGS.put(TYPE_STRAIGHT_AC, "T_SA");
        INTS_TO_SHORT_STRINGS.put(TYPE_STRAIGHT, "T_ST");
        // complete situations when on the track
        INTS_TO_SHORT_STRINGS.put(STRAIGHT, "STR");
        INTS_TO_SHORT_STRINGS.put(STRAIGHT_AC, "SAC");
        INTS_TO_SHORT_STRINGS.put(FULL_RIGHT, "FR");
        INTS_TO_SHORT_STRINGS.put(FULL_LEFT, "FL");
        INTS_TO_SHORT_STRINGS.put(MEDIUM_RIGHT, "MR");
        INTS_TO_SHORT_STRINGS.put(MEDIUM_LEFT, "ML");
        INTS_TO_SHORT_STRINGS.put(SLOW_RIGHT, "SR");
        INTS_TO_SHORT_STRINGS.put(SLOW_LEFT, "SL");
        INTS_TO_SHORT_STRINGS.put(HAIRPIN_LEFT, "HL");
        INTS_TO_SHORT_STRINGS.put(HAIRPIN_RIGHT, "HR");
//
//        Set<Integer> keys = INTS_TO_STRINGS.keySet();
//        for(Integer i : keys){
//            System.out.println(Integer.toHexString(i)+" "+INTS_TO_STRINGS.get(i));
//        }
        

        //INTS_TO_STRINGS.put(, "");

        MIRROR.put(STRAIGHT, STRAIGHT);
        MIRROR.put(STRAIGHT_AC, STRAIGHT_AC);
        //MIRROR.put(LEFT_HAIRPIN_ENTRY, RIGHT_HAIRPIN_ENTRY);
        //MIRROR.put(LEFT_HAIRPIN_EXIT, RIGHT_HAIRPIN_EXIT);
        //MIRROR.put(RIGHT_HAIRPIN_ENTRY, LEFT_HAIRPIN_ENTRY);
        //MIRROR.put(RIGHT_HAIRPIN_EXIT, LEFT_HAIRPIN_EXIT);
        MIRROR.put(FULL_RIGHT, FULL_LEFT);
        MIRROR.put(FULL_LEFT, FULL_RIGHT);
        MIRROR.put(MEDIUM_LEFT, MEDIUM_RIGHT);
        MIRROR.put(MEDIUM_RIGHT, MEDIUM_LEFT);
        MIRROR.put(SLOW_LEFT, SLOW_RIGHT);
        MIRROR.put(SLOW_RIGHT, SLOW_LEFT);
        MIRROR.put(HAIRPIN_RIGHT, HAIRPIN_LEFT);
        MIRROR.put(HAIRPIN_LEFT, HAIRPIN_RIGHT);
        //MIRROR.put(, );

        SITUATIONS_AS_STRINGS.add("STRAIGHT");
        SITUATIONS_AS_STRINGS.add("STRAIGHT_AC");
        SITUATIONS_AS_STRINGS.add("FULL_RIGHT");
        SITUATIONS_AS_STRINGS.add("FULL_LEFT");
        SITUATIONS_AS_STRINGS.add("MEDIUM_RIGHT");
        SITUATIONS_AS_STRINGS.add("MEDIUM_LEFT");
        SITUATIONS_AS_STRINGS.add("SLOW_RIGHT");
        SITUATIONS_AS_STRINGS.add("SLOW_LEFT");
        SITUATIONS_AS_STRINGS.add("HAIRPIN_LEFT");
        SITUATIONS_AS_STRINGS.add("HAIRPIN_RIGHT");
    }



    
    /**
     * This class is not meant to be instantiated.
     */
    private Situations() {
    }

    public static int fromString(String s) {
        Integer i = STRING_TO_INTS.get(s);

        if(i != null){
            return i.intValue();
            
        } else {
            throw new RuntimeException(s + " is not a valid identifier.");
        }

    }

    public static String toString(int i) {
        String s = INTS_TO_STRINGS.get(i);

        if(s != null){
            return s;
            
        } else {
            throw new RuntimeException(String.valueOf(i) + " is not a valid identifier.");
        }
    }

    public static String toShortString(int i) {
        String s = INTS_TO_SHORT_STRINGS.get(i);

        if(s != null){
            return s;

        } else {
            throw new RuntimeException(String.valueOf(i) + " is not a valid identifier.");
        }
    }

    public static int mirror(int i){
        Integer result = MIRROR.get(i);

        if(result != null){
            return result.intValue();

        } else {
            throw new RuntimeException(i + " is not a valid situation identifier.");
        }
    }    

    public static Iterator<String> situations(){
        return SITUATIONS_AS_STRINGS.iterator();
    }

    public static void main(String[] args){
        dumpTable(STRING_TO_INTS);
    }

    private static void dumpTable(HashMap<String, Integer> map){
        Set<String> set = map.keySet();        
        for(String s: set){
            System.out.println(s+" - "+map.get(s));
        }
    }

    public static int fiveTypesDistance(int t1, int t2){
        int index1 = -1;
        int index2 = -1;
        for(int i=0; i < FIVE_TYPES.length; ++i){
            if(FIVE_TYPES[i] == t1){
                index1 = i;
            }
            if(FIVE_TYPES[i] == t2){
                index2 = i;
            }
        }
        return Math.abs(index1-index2);
    }
}
