package de.janquadflieg.mrracer.evo;

public class EvoResults
        implements Comparable<EvoResults> {

    public double damage;
    public double distance;
    public int offTrack;

    public double getFitness() {
        // original
        return distance - ((damage / 25) + (offTrack / 25));
        // test 1
        //return distance - ((damage * 5) + (offTrack * 5));
        // test 2
        //return distance - ((damage / 500) + (offTrack / 500));
    }

    public void set(EvoResults r) {
        this.damage = r.damage;
        this.offTrack = r.offTrack;
        this.distance = r.distance;
    }

    @Override
    public int compareTo(EvoResults t) {
        if (getFitness() < t.getFitness()) {
            return -1;
        } else if (getFitness() > t.getFitness()) {
            return 1;
        } else {
            return 0;
        }
    }
}
