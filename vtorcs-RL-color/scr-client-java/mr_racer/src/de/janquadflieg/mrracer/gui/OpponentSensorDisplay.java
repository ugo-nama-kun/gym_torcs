/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.gui;

import de.janquadflieg.mrracer.Utils;
import de.janquadflieg.mrracer.telemetry.SensorData;
import de.janquadflieg.mrracer.telemetry.ModifiableSensorData;

import java.awt.*;
import java.awt.geom.*;
import javax.swing.*;

/**
 *
 * @author Jan Quadflieg
 */
public class OpponentSensorDisplay
        extends JComponent {

    private ModifiableSensorData m = new ModifiableSensorData();
    private static final int CAR_WIDTH = 2;
    private static final int CAR_LENGTH = 5;
    /** Pixel per meter. */
    private static final int PPM = 2;

    public OpponentSensorDisplay() {
    }

    @Override
    public void paintComponent(Graphics graphics) {
        Graphics2D g = (Graphics2D) graphics;

        g.setColor(Color.WHITE);

        g.fillRect(0, 0, getWidth(), getHeight());

        if (m.getAngleToTrackAxis() != Utils.NO_DATA_D) {
            // my car
            drawCar(g);

            double[] sensors = m.getOpponentSensors();
            boolean allHundred = true;
            for (int i = 0; i < sensors.length; ++i) {
                allHundred &= (sensors[i] == 200.0);
            }

            if (allHundred) {
                g.setColor(Color.BLACK);
                g.drawString("No opponent in sight", 30, 30);

            } else {
                // track sensors
                drawOpponentSensors(g);
            }

        } else {
            g.setColor(Color.BLACK);
            g.drawString("No data available", 30, 30);
        }
    }

    private void drawCar(Graphics2D g) {
        AffineTransform backup = g.getTransform();

        g.translate(getWidth() / 2, (getHeight() - (CAR_LENGTH * PPM)) / 2);
        g.setColor(Color.BLACK);
        g.fillRect(-(CAR_WIDTH / 2), 0, CAR_WIDTH, CAR_LENGTH);

        g.setTransform(backup);
    }

    private void drawOpponentSensors(Graphics2D g) {
        AffineTransform backup = g.getTransform();

        g.translate(getWidth() / 2, (getHeight() - (CAR_LENGTH * PPM)) / 2);

        g.setColor(Color.RED);

        double[] sensors = m.getOpponentSensors();

        int last_x = 0;
        int last_y = 0;

        for (int i = 0; i < sensors.length; ++i) {
            if (sensors[i] < 200.0) {
                double angle = Math.toRadians(-180.0) + Math.toRadians(i * 10.0);
                int x_offset = (int) Math.round(Math.sin(angle) * sensors[i] * PPM);
                int y_offset = (int) Math.round(-Math.cos(angle) * sensors[i] * PPM);


                g.setColor(Color.RED);
                g.drawLine(0, 0, x_offset, y_offset);
                g.fillRect(x_offset - 1, y_offset - 1, 2, 2);
                g.setColor(Color.BLUE);
                g.drawString(Integer.toString(i)+"-"+String.format("%.1f", sensors[i]),
                        x_offset + 2, y_offset + 2);


                //if (i > 0) {
                //    g.setColor(Color.RED);
                //    g.drawLine(last_x, last_y, x_offset, y_offset);
                //}

//                last_x = x_offset;
//                last_y = y_offset;
            }
        }

        g.setTransform(backup);
    }

    public void setSensorData(SensorData model) {
        this.m.setData(model);
        repaint();
    }
}
