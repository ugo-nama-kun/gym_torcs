package de.delbrueg.steering.behaviour.decision;

/**
 *
 * @author Tim
 */
public class ConstantDecider extends LineOrCircleDecider {

    private boolean linear;

    public ConstantDecider(boolean linear) {
        this.linear = linear;
    }

    @Override
    public boolean isLinear() {

        return linear;
    }
}
