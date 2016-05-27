/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.track;

import de.janquadflieg.mrracer.Utils;
import de.janquadflieg.mrracer.classification.AngleBasedClassifier;
import de.janquadflieg.mrracer.classification.Situation;
import de.janquadflieg.mrracer.classification.Situations;
import de.janquadflieg.mrracer.telemetry.SensorData;

import java.util.*;

/**
 * A track segment is one segment, which has been classfied as one consistent
 * direction.  It ususally consists of one or more subsegments.
 *
 * @author Jan Quadflieg
 */
public class TrackSegment
        implements java.io.Serializable {

    static final long serialVersionUID = -458522181630174539L;
    /** Flag indicating if this segment represents an unknown part of the track. */
    private boolean unknown = false;
    /** Start point of this segment. */
    private double start;
    /** Length of this segment. */
    private double length = 0;
    /** Direction of this segment. */
    private int direction = 0;
    /** Width. */
    private double width = 0.0;
    /** Sub segments. */
    private ArrayList<TrackSubSegment> segments = new ArrayList<TrackSubSegment>();
    /** Filtered? */
    private boolean filtered = false;
    protected double integralPosition = 0.0;
    protected double zeroMiddlePosition = 0.0;
    public static final double DEFAULT_SPEED = Double.MIN_VALUE;
    private double[] targetSpeeds = null;

    private TrackSegment() {
    }

    public TrackSegment(double start, double end, double width) {
        this.start = start;
        this.length = Math.max(end - start, Double.MIN_NORMAL);
        unknown = true;
        this.width = width;
    }

    public TrackSegment(double start, double end, Situation situation, double width) {
        this.start = start;
        this.length = Math.max(end - start, Double.MIN_NORMAL);
        this.direction = situation.getDirection();
        this.width = width;
        int type = situation.getType();
        if (type == Situations.TYPE_STRAIGHT_AC) {
            type = Situations.TYPE_STRAIGHT;
        }
        segments.add(new TrackSubSegment(start, end, situation));
    }

    private void checkIntegrity() {
        if (unknown || direction == Situations.DIRECTION_FORWARD) {
            return;
        }
        if (segments.get(0).getType() != Situations.TYPE_FULL) {
            TrackSubSegment s = segments.get(0);
            //System.out.println("WARNING: 0-Not Full "+subToString(s));
            double l = s.getLength();
            int type = s.getType();
            if (type == Situations.TYPE_MEDIUM) {
                l = l / 2.0;
                s.setStart(start + l);
                segments.add(0, new TrackSubSegment(this.start,
                        this.start + l, Situations.TYPE_FULL,
                        AngleBasedClassifier.FULL_MEDIUM - 1.0));
                segments.get(0).setMaxPosition(this.start + l);
            //System.out.println("...fixed");

            } else if (type == Situations.TYPE_SLOW) {
                l = l / 3.0;
                s.setStart(start + l + l);
                segments.add(0, new TrackSubSegment(this.start + l,
                        this.start + l + l, Situations.TYPE_MEDIUM,
                        AngleBasedClassifier.MEDIUM_SLOW - 1.0));
                segments.get(0).setMaxPosition(this.start + l + l);

                segments.add(0, new TrackSubSegment(this.start,
                        this.start + l, Situations.TYPE_FULL,
                        AngleBasedClassifier.FULL_MEDIUM - 1.0));
                segments.get(0).setMaxPosition(this.start + l);
            //System.out.println("...fixed");

            } else if (type == Situations.TYPE_HAIRPIN) {
                l = l / 4.0;
                s.setStart(start + l + l + l);
                segments.add(0, new TrackSubSegment(this.start + l + l,
                        this.start + l + l + l, Situations.TYPE_SLOW,
                        AngleBasedClassifier.SLOW_HAIRPIN - 1.0));
                segments.get(0).setMaxPosition(this.start + l + l + l);

                segments.add(0, new TrackSubSegment(this.start + l,
                        this.start + l + l, Situations.TYPE_MEDIUM,
                        AngleBasedClassifier.MEDIUM_SLOW - 1.0));
                segments.get(0).setMaxPosition(this.start + l + l);
                segments.add(0, new TrackSubSegment(this.start,
                        this.start + l, Situations.TYPE_FULL,
                        AngleBasedClassifier.FULL_MEDIUM - 1.0));
                segments.get(0).setMaxPosition(this.start + l);
            //System.out.println("...fixed");

            } else {
                //System.out.println("WARNING: 0-Not Full " + subToString(s));
                //System.out.println("NOT IMPLEMENTED");
            }
        }
        for (int i = 1; i < segments.size(); ++i) {
            TrackSubSegment s1 = segments.get(i - 1);
            TrackSubSegment s2 = segments.get(i);
            int t1 = s1.getType();
            int t2 = s2.getType();

            if (Situations.fiveTypesDistance(t1, t2) == 0) {
                //System.out.println("WARNING [0]: " + subToString(s1)
                //        + " - " + subToString(s2));
            } else if (Situations.fiveTypesDistance(t1, t2) == 2) {
                //System.out.println("WARNING [2]: " + subToString(s1) +
                //        " - " + subToString(s2));
                int typeInsert = Situations.TYPE_MEDIUM;
                double maxMeasure = AngleBasedClassifier.MEDIUM_SLOW - 1.0;
                if ((t1 == Situations.TYPE_MEDIUM && t2 == Situations.TYPE_HAIRPIN) || (t2 == Situations.TYPE_MEDIUM && t1 == Situations.TYPE_HAIRPIN)) {
                    typeInsert = Situations.TYPE_SLOW;
                    maxMeasure = AngleBasedClassifier.SLOW_HAIRPIN - 1.0;
                }
                double l1 = segments.get(i - 1).getLength();
                double l2 = segments.get(i).getLength();
                double l = Math.min(l1, l2);
                if (l1 == l2) {
                    l = l / 2.0;
                }
                //System.out.println("Inserting "+Situations.toShortString(typeInsert)+", "+l+"m");

                if (l1 < l2) {
                    s2.setStart(s2.getStart() + l);
                } else {
                    s1.setEnd(s1.getEnd() - l);
                }
                segments.add(i, new TrackSubSegment(s1.getEnd(), s2.getStart(), typeInsert, maxMeasure));
            //System.out.println("...fixed: "+subToString(s1)+" - "+
            //      subToString(segments.get(i))+" - "+subToString(s2));


            } else if (Situations.fiveTypesDistance(t1, t2) == 3) {
                /*System.out.println("WARNING [3]: " + subToString(segments.get(i - 1)) +
                " - " + subToString(segments.get(i)));*/
                double l1 = segments.get(i - 1).getLength();
                double l2 = segments.get(i).getLength();
                double l = Math.min(l1, l2);
                if (l1 == l2) {
                    l = l / 3.0;
                }

                if (l1 < l2) {
                    s2.setStart(s2.getStart() + l + l);
                } else {
                    s1.setEnd(s1.getEnd() - (l + l));
                }

                if (t1 == Situations.TYPE_FULL) {
                    segments.add(i, new TrackSubSegment(s1.getEnd() + l,
                            s2.getStart(), Situations.TYPE_SLOW,
                            AngleBasedClassifier.SLOW_HAIRPIN - 1.0));
                    segments.get(i).setMaxPosition(s2.getStart());
                    segments.add(i, new TrackSubSegment(s1.getEnd(),
                            s2.getStart() - l, Situations.TYPE_MEDIUM,
                            AngleBasedClassifier.MEDIUM_SLOW - 1.0));
                    segments.get(i).setMaxPosition(s2.getStart() - l);

                } else {
                    segments.add(i, new TrackSubSegment(s1.getEnd() + l,
                            s2.getStart(), Situations.TYPE_MEDIUM,
                            AngleBasedClassifier.MEDIUM_SLOW - 1.0));
                    segments.get(i).setMaxPosition(s1.getEnd() + l);

                    segments.add(i, new TrackSubSegment(s1.getEnd(),
                            s2.getStart() - l, Situations.TYPE_SLOW,
                            AngleBasedClassifier.SLOW_HAIRPIN - 1.0));
                    segments.get(i).setMaxPosition(s1.getEnd());
                }

            /*System.out.println("...fixed: "+subToString(s1)+" - "+
            subToString(segments.get(i))+" - "+subToString(segments.get(i+1))+
            " - "+subToString(s2));*/



            } else if (Situations.fiveTypesDistance(t1, t2) != 1) {
                //System.out.println("WARNING [>3]: " + subToString(segments.get(i - 1))
                //        + " - " + subToString(segments.get(i)));
                //System.out.println("NOT IMPLEMENTED");
            }
        }
        if (segments.get(segments.size() - 1).getType() != Situations.TYPE_FULL) {
            TrackSubSegment s = segments.get(segments.size() - 1);
            //System.out.println("WARNING: Last-Not Full "+subToString(s));
            double l = s.getLength();
            int type = s.getType();
            if (type == Situations.TYPE_MEDIUM) {
                l = l / 2.0;
                s.setEnd(getEnd() - l);
                segments.add(new TrackSubSegment(this.getEnd() - l,
                        this.getEnd(), Situations.TYPE_FULL,
                        AngleBasedClassifier.FULL_MEDIUM - 1.0));
                segments.get(segments.size() - 1).setMaxPosition(this.getEnd() - l);
            //System.out.println("...fixed");

            } else if (type == Situations.TYPE_SLOW) {
                l = l / 3.0;
                s.setEnd(getEnd() - (l + l));
                segments.add(new TrackSubSegment(this.getEnd() - (l + l),
                        this.getEnd() - l, Situations.TYPE_MEDIUM,
                        AngleBasedClassifier.MEDIUM_SLOW - 1.0));
                segments.get(segments.size() - 1).setMaxPosition(this.getEnd() - (l + l));
                segments.add(new TrackSubSegment(this.getEnd() - l,
                        this.getEnd(), Situations.TYPE_FULL,
                        AngleBasedClassifier.FULL_MEDIUM - 1.0));
                segments.get(segments.size() - 1).setMaxPosition(this.getEnd() - l);
            //System.out.println("...fixed");

            } else if (type == Situations.TYPE_HAIRPIN) {
                l = l / 4.0;
                s.setEnd(getEnd() - (l + l + l));
                segments.add(new TrackSubSegment(this.getEnd() - (l + l + l),
                        this.getEnd() - (l + l), Situations.TYPE_SLOW,
                        AngleBasedClassifier.SLOW_HAIRPIN));
                segments.get(segments.size() - 1).setMaxPosition(this.getEnd() - (l + l + l));
                segments.add(new TrackSubSegment(this.getEnd() - (l + l),
                        this.getEnd() - l, Situations.TYPE_MEDIUM,
                        AngleBasedClassifier.MEDIUM_SLOW - 1.0));
                segments.get(segments.size() - 1).setMaxPosition(this.getEnd() - (l + l));
                segments.add(new TrackSubSegment(this.getEnd() - l,
                        this.getEnd(), Situations.TYPE_FULL,
                        AngleBasedClassifier.FULL_MEDIUM - 1.0));
                segments.get(segments.size() - 1).setMaxPosition(this.getEnd() - l);
            //System.out.println("...fixed");

            } else {
                // should not happen
                //System.out.println("NOT IMPLEMENTED");
            }
        }
    }

    public boolean contains(double d) {
        return d >= this.start && d < this.getEnd();
    }

    protected void extendBack(double end) {
        if (unknown) {
            throw new RuntimeException();
        }
        segments.get(segments.size() - 1).setEnd(end);
        this.length = end - start;
    }

    protected void extendFront(double newStart) {
        if (unknown) {
            throw new RuntimeException();
        }
        segments.get(0).setStart(newStart);
        this.length += start - newStart;
        this.start = newStart;
    }

    protected void extendFront(double newStart, Situation s) {
        if (unknown) {
            throw new RuntimeException();
        }

        if (segments.size() == 1 && segments.get(0).getType() == Situations.TYPE_STRAIGHT) {
            segments.get(0).setStart(newStart);

        } else {
            TrackSubSegment first = segments.get(0);
            if (first.getType() == s.getType()) {
                first.setStart(newStart);

            } else {
                segments.add(0, new TrackSubSegment(newStart, start,
                        s));
            }
        }

        this.length += start - newStart;
        this.start = newStart;
    }

    protected void extendBack(double end, Situation s) {
        if (unknown) {
            throw new RuntimeException();
        }
        if (segments.size() == 1 && segments.get(0).getType() == Situations.TYPE_STRAIGHT) {
            segments.get(0).setEnd(end, s);

        } else {
            TrackSubSegment predecessor = segments.get(segments.size() - 1);
            if (predecessor.getType() == s.getType()) {
                predecessor.setEnd(end, s);

            } else {
                segments.add(new TrackSubSegment(start + length, end,
                        s));
            }
        }

        this.length = end - this.start;
    }

    protected void filter() {
        filter(false);
    }

    protected void filter(boolean noisy) {
        if(unknown){
            if (TrackModel.TEXT_DEBUG) {
                System.out.println("WARNING: filter() called on unknown segment");
            }
            return;
        }
        if (filtered) {
            if (TrackModel.TEXT_DEBUG) {
                System.out.println("Allready filtered");
            }
            return;
        }
        checkIntegrity();

        if (segments.size() < 3) {
            if (TrackModel.TEXT_DEBUG) {
                System.out.println("Only " + segments.size() + " sub segments, nothing to do");
            }
            filtered = true;
            return;
        }

        // remove small faster subsegments which create unnecessary multi-apex corners
        for (int i = 2; i < segments.size(); ++i) {
            if (segments.get(i - 2).getType() == segments.get(i).getType() &&
                    segments.get(i - 2).getType() < segments.get(i - 1).getType() &&
                    segments.get(i - 1).getLength() < (2 * width)) {

                double newEnd = segments.get(i).getEnd();

                for (int k = i - 1; k <= i; ++k) {
                    if (segments.get(k).getMaximum() > segments.get(i - 2).getMaximum()) {
                        segments.get(i - 2).setMaxPosition(segments.get(k).getMaxPosition());
                        segments.get(i - 2).setMaximum(segments.get(k).getMaximum());
                    }
                }

                if (TrackModel.TEXT_DEBUG) {
                    System.out.println("Removing small faster ss[" + (i - 1) + "/" +
                            Situations.toShortString(segments.get(i - 1).getType()) +
                            "], outer: " + Situations.toShortString(segments.get(i).getType()));
                }

                segments.get(i - 2).setEnd(newEnd);
                segments.remove(i - 1);
                segments.remove(i - 1);
                i = 1;
            }
        }

        //System.out.println("***********   FILTER     ************************");
        // slower subsegments which are too short, makes a corner faster
        if (TrackModel.TEXT_DEBUG) {
            System.out.println("Checking slower sub segments");
        }
        for (int i = 2; i < segments.size(); ++i) {
            if (segments.get(i - 2).getType() == segments.get(i).getType() && segments.get(i - 2).getType() > segments.get(i - 1).getType()) {
                double factor = segments.get(i - 1).getFilterFactor();
                double fraction = segments.get(i - 1).getLength() / width;
                double maxValue = segments.get(i - 1).getMaximum();
                int type = segments.get(i - 1).getType();
                boolean remove = false;

                //System.out.println("fraction: "+fraction);
                //System.out.println("factor: "+factor);
                //System.out.println("2.0 / factor: "+(2.0 / factor));

                if (type == Situations.TYPE_HAIRPIN && fraction < 2.0) {
                    if (TrackModel.TEXT_DEBUG) {
                        System.out.println("Remove hairpin ss, fraction: "+fraction);
                    }
                    remove = true;
                    factor *= 4.0;
                }
                if (type == Situations.TYPE_SLOW && fraction < 1.5 / factor) {
                    if (TrackModel.TEXT_DEBUG) {
                        System.out.println("Remove slow ss, fraction: "+fraction+
                                ", 1.5/factor: "+(1.5 / factor));
                    }                    
                    remove = true;
                    factor *= 2.0;
                }
                if (type == Situations.TYPE_MEDIUM && fraction < 2.0 / factor) {
                    if (TrackModel.TEXT_DEBUG) {
                        System.out.println("Remove medium ss, fraction: "+fraction+
                                ", 2.0/factor: "+(2.0 / factor));
                    }                    
                    remove = true;
                }
                if (noisy && type == Situations.TYPE_MEDIUM && maxValue <
                        24.0 && fraction < 1.0) {
                    if (TrackModel.TEXT_DEBUG) {
                        System.out.println("Remove medium ss, fraction: "+fraction+
                                ", maxValue: "+maxValue);
                    }                    
                    remove = true;
                }
                if (type == Situations.TYPE_MEDIUM && width > 15 && fraction > 10.0 &&
                        maxValue < 30.0) {
                    if (TrackModel.TEXT_DEBUG) {
                        System.out.println("Remove medium on wide track, fraction: "+fraction+
                                ", maxValue: "+maxValue+", width: "+width);
                    }
                    remove = true;
                }


                if (remove) {
                    if (TrackModel.TEXT_DEBUG) {
                        System.out.println("Removing small slower ss[" + (i - 1) + "/" +
                                Situations.toShortString(type));
                    }

                    //System.out.println("FILTER: removing with factor "+segments.get(i-1).getFilterFactor() + subToString(segments.get(i - 1)));
                    //System.out.println(this);
                    double newEnd = segments.get(i).getEnd();
                    segments.get(i - 2).setEnd(newEnd);
                    segments.get(i - 2).setFilterFactor(factor);

                    for (int k = i - 1; k <= i; ++k) {
                        if (segments.get(k).getMaximum() > segments.get(i - 2).getMaximum()) {
                            segments.get(i - 2).setMaxPosition(segments.get(k).getMaxPosition());
                            segments.get(i - 2).setMaximum(segments.get(k).getMaximum());
                        }
                    }

                    segments.remove(i - 1);
                    segments.remove(i - 1);
                    i = 1;
                }
            }
        }

        //System.out.println(this);
        //System.out.println("");
        filtered = true;
    }

    protected boolean filtered(){
        return filtered;
    }

    public Apex[] getApexes() {
        if (unknown) {
            Apex apex = new Apex();
            apex.position = this.start + (this.length / 2.0);
            apex.type = Situations.TYPE_HAIRPIN;
            apex.unknown = true;
            apex.index = 0;

            return new Apex[]{apex};
        }
        if (segments.size() == 1) {
            Apex apex = new Apex();
            apex.naivPosition = this.start + (this.length / 2.0);
            apex.integralPosition = this.integralPosition;
            apex.zeroMiddlePosition = this.zeroMiddlePosition;
            apex.position = apex.naivPosition;

            apex.type = getType();
            apex.index = 0;
            apex.value = segments.get(0).getMaximum();

            if (targetSpeeds != null && targetSpeeds.length > 0) {
                apex.targetSpeed = targetSpeeds[0];
            }

            return new Apex[]{apex};
        }
        ArrayList<Apex> list = new ArrayList<Apex>();
        for (int i = 2; i < segments.size(); ++i) {
            int s0 = segments.get(i - 2).getType();
            int s1 = segments.get(i - 1).getType();
            int s2 = segments.get(i).getType();

            boolean isApex = s0 > s1 && s1 < s2;
            if (isApex) {
                TrackSubSegment s = segments.get(i - 1);
                Apex apex = new Apex();
                apex.naivPosition = s.getStart() + (s.getLength() / 2.0);
                apex.integralPosition = apex.naivPosition;
                apex.zeroMiddlePosition = apex.naivPosition;

                if (s.contains(this.integralPosition)) {
                    apex.integralPosition = this.integralPosition;
                }
                if (s.contains(this.zeroMiddlePosition)) {
                    apex.zeroMiddlePosition = this.zeroMiddlePosition;
                }

                apex.position = apex.integralPosition;

                apex.value = s.getMaximum();
                apex.type = s.getType();
                apex.index = i;

                list.add(apex);
            }
        }
        if (list.isEmpty()) {
            Apex apex = new Apex();
            apex.position = this.start + (this.length / 2.0);
            apex.type = getType();
            apex.index = 0;

            return new Apex[]{apex};

        } else {
            Apex[] result = new Apex[list.size()];

            for (int i = 0; i < result.length; ++i) {
                result[i] = list.get(i);
                result[i].index = i;
                if (targetSpeeds != null && targetSpeeds.length > i) {
                    result[i].targetSpeed = targetSpeeds[i];
                }
            }

            return result;
        }
    }

    public int getDirection() {
        return this.direction;
    }

    public double getEnd() {
        return start + length;
    }

    /**
     * Return the index of the subsegment which contains pos.
     * @param pos Unit: DistanceToStartLine
     * @return Index of the subsegment.
     */
    public int getIndex(double pos) {
        for (int i = 0; i < segments.size(); ++i) {
            if (pos >= segments.get(i).getStart() && pos < segments.get(i).getEnd()) {
                return i;
            }
        }
        return size() - 1;
    }

    public double getLength() {
        return length;
    }

    public int getType() {
        if (unknown || segments.isEmpty()) {
            return Situations.TYPE_OUTSIDE;

        } else if (segments.size() == 1) {
            return segments.get(0).getType();

        } else {
            int result = Situations.TYPE_FULL;
            for (int i = 0; i < segments.size(); ++i) {
                TrackSubSegment s = segments.get(i);
                result = Math.min(result, s.getType());
            }

            return result;
        }
    }

    public TrackSubSegment getSubSegment(double d) {
        return segments.get(getIndex(d));
    }

    public TrackSubSegment getSubSegment(int i) {
        return segments.get(i);
    }

    public double getStart() {
        return start;
    }

    protected void initTargetSpeeds() {
        TrackSegment.Apex[] apexes = getApexes();
        targetSpeeds = new double[apexes.length];
        for (int i = 0; i < targetSpeeds.length; ++i) {
            targetSpeeds[i] = DEFAULT_SPEED;
        }
    }

    public boolean isRight() {
        return this.getDirection() == Situations.DIRECTION_RIGHT && !unknown;
    }

    public boolean isLeft() {
        return this.getDirection() == Situations.DIRECTION_LEFT && !unknown;
    }

    public boolean isFull() {
        return this.getType() == Situations.TYPE_FULL && !unknown;
    }

    public boolean isCorner() {
        return this.direction != Situations.DIRECTION_FORWARD && !unknown;
    }

    public boolean isForward() {
        return !unknown && this.direction == Situations.DIRECTION_FORWARD;
    }

    public boolean isStraight() {
        return !unknown && this.direction == Situations.DIRECTION_FORWARD;
    }

    public boolean isUnknown() {
        return unknown;
    }

    protected void join(TrackSegment s) {
        if (this.direction != s.direction) {
            throw new RuntimeException();
        }

        if (this.segments.get(this.segments.size() - 1).getType() == s.segments.get(0).getType()) {
            this.segments.get(this.segments.size() - 1).setEnd(s.segments.get(0).getEnd());

            for (int i = 1; i < s.segments.size(); ++i) {
                this.segments.add(s.segments.get(i));
            }

        } else {
            double middle = this.getEnd() + ((s.getStart() - this.getEnd()) / 2.0);
            this.segments.get(this.segments.size() - 1).setEnd(middle);
            s.segments.get(0).setStart(middle);

            for (int i = 0; i < s.segments.size(); ++i) {
                this.segments.add(s.segments.get(i));
            }
        }

        this.length = s.getEnd() - start;
    }

    public void setTargetSpeed(int i, double speed) {
        targetSpeeds[i] = speed;
    }

    protected void setWidth(double w) {
        this.width = w;
    }

    protected void shortenBack(double end) {
        if (!unknown) {
            throw new RuntimeException();
        }
        this.length = end - start;
    }

    protected void shortenFront(double newStart) {
        if (!unknown) {
            throw new RuntimeException();
        }
        double oldEnd = start + length;
        this.start = newStart;
        this.length = oldEnd - start;
    }

    protected void toStraight() {
        if (this.unknown || !this.isFull()) {
            throw new RuntimeException();
        }
        this.direction = Situations.DIRECTION_FORWARD;
        this.segments.get(0).setType(Situations.TYPE_STRAIGHT);
    }

    public void write(java.io.OutputStreamWriter w, int version)
            throws Exception {
        w.write(String.valueOf(start));
        w.write(" ");
        w.write(String.valueOf(length));
        w.write(" ");
        w.write(String.valueOf(unknown));
        w.write(" ");
        w.write(String.valueOf(direction));
        w.write(" ");
        w.write(String.valueOf(segments.size()));
        w.write(" ");
        for (TrackSubSegment subseg : segments) {
            w.write(String.valueOf(subseg.getStart()));
            w.write(" ");
            w.write(String.valueOf(subseg.getLength()));
            w.write(" ");
            w.write(String.valueOf(subseg.getType()));
            w.write(" ");
            if (version > 1) {
                w.write(String.valueOf(subseg.getMaximum()));
                w.write(" ");
                w.write(String.valueOf(subseg.getMaxPosition()));
                w.write(" ");
            }
        }
        if (version > 2 && targetSpeeds != null) {
            for (int i = 0; i < targetSpeeds.length; ++i) {
                w.write(String.valueOf(targetSpeeds[i]));
                w.write(" ");
            }
        }
    }

    public static TrackSegment read(String s, int version, boolean modelComplete)
            throws Exception {
        TrackSegment result = new TrackSegment();
        StringTokenizer tokenizer = new StringTokenizer(s, " ");
        result.start = Double.parseDouble(tokenizer.nextToken());
        result.length = Double.parseDouble(tokenizer.nextToken());
        result.unknown = Boolean.parseBoolean(tokenizer.nextToken());
        result.direction = Integer.parseInt(tokenizer.nextToken());
        int numSubSegments = Integer.parseInt(tokenizer.nextToken());
        for (int i = 0; i < numSubSegments; ++i) {
            double start = Double.parseDouble(tokenizer.nextToken());
            double length = Double.parseDouble(tokenizer.nextToken());
            int type = Integer.parseInt(tokenizer.nextToken());
            double max = 0;
            double maxPos = start + (length / 2.0);

            if (version > 1) {
                max = Double.parseDouble(tokenizer.nextToken());
                maxPos = Double.parseDouble(tokenizer.nextToken());
            }

            TrackSubSegment subseg = new TrackSubSegment(start, start + length, type, max);
            subseg.setMaxPosition(maxPos);

            result.segments.add(subseg);
        }

        if (modelComplete) {
            result.initTargetSpeeds();
            result.filtered = true;
            if (version > 2 && tokenizer.hasMoreTokens()) {
                for (int i = 0; i < result.targetSpeeds.length; ++i) {
                    result.targetSpeeds[i] = Double.parseDouble(tokenizer.nextToken());
                }
            }
        }

        return result;
    }

    public int size() {
        return segments.size();
    }

    @Override
    public String toString() {
        return toString(false);
    }
    
    public String toString(boolean linebreak) {
        if (unknown) {
            return String.format("TrackSegment %10.2f %10.2f %10.2f unknown",
                    start, start + length, length);
        } else {
            String result = String.format("TrackSegment %10.2f %10.2f %10.2f %3s %3s",
                    start, start + length, length, Situations.toShortString(direction),
                    Situations.toShortString(getType()));

            for (TrackSubSegment subseg : segments) {
                if(linebreak){
                   result += "\n   ";
                }
                result += subToString(subseg);
            }

            if (isCorner()) {
                TrackSegment.Apex[] apexes = getApexes();
                for (TrackSegment.Apex a : apexes) {
                    result += "\n   ";
                    result += a.toString();
                }
            }

            return result;
        }
    }

    private String subToString(TrackSubSegment subseg) {
        double fraction = (subseg.getLength() / this.length) * 100.0;
        double wfraction = (subseg.getLength() / this.width) * 100.0;
        return String.format(" [%.1f %.1f %5.1f%% %7.1f%% %4s [%.1f:%.1f]]", subseg.getStart(),
                subseg.getLength(), fraction, wfraction, Situations.toShortString(subseg.getType()),
                subseg.getMaxPosition(), subseg.getMaximum());
    }

    public static class Apex {

        public double position = 0.0;
        public double naivPosition = 0.0;
        public double integralPosition = 0.0;
        public double zeroMiddlePosition = 0.0;
        public int type = 0;
        public int index = 0;
        public double value = 0.0;
        public boolean unknown = false;
        public double targetSpeed = TrackSegment.DEFAULT_SPEED;

        public String toString() {
            String result = "Apex[" + index + "] " + Utils.dTS(position) + "m, " + Situations.toShortString(type) + ", " + Utils.dTS(value) + ", v=";
            if (this.targetSpeed == TrackSegment.DEFAULT_SPEED) {
                result += "default";
            } else {
                result += Utils.dTS(this.targetSpeed);
            }
            return result;
        }
    }
}
