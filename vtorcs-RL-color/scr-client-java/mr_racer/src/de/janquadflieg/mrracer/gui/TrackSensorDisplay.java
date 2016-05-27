/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package de.janquadflieg.mrracer.gui;

import flanagan.analysis.Regression;

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
public class TrackSensorDisplay
        extends JComponent {

    private static final Color DARK_GREEN = new Color(30, 181, 0);
    private static final Color PURPLE = new Color(0xbd0ebf);
    private ModifiableSensorData m = new ModifiableSensorData();
    private static final int CAR_WIDTH = 2;
    private static final int CAR_LENGTH = 5;
    /** Pixel per meter. */
    private static final int PPM = 3;
    private boolean drawSensors = true;
    private boolean drawFocus = true;
    private boolean drawOutline = true;
    private boolean drawCoordinates = true;
    private boolean drawCapOff = true;
    private boolean drawFiltered = true;
    private double[] angles = new double[19];
    private int focusAngle = 0;
    private double[] focusSensors = new double[5];

    public TrackSensorDisplay() {
        for (int i = 0; i < 19; ++i) {
            angles[i] = -90 + i * 10;
        }
    }

    public void setAngles(float[] angles) {
        for (int i = 0; i < 19; ++i) {
            this.angles[i] = angles[i];
        }
    }

    @Override
    public void paintComponent(Graphics graphics) {
        Graphics2D g = (Graphics2D) graphics;

        g.setColor(Color.WHITE);

        g.fillRect(0, 0, getWidth(), getHeight());

        if (m.getAngleToTrackAxis() != Utils.NO_DATA_D) {
            // track sensors
            drawTrackSensors(g);

        // my car
        //drawCar(g);

        } else {
            g.setColor(Color.BLACK);
            g.drawString("No data available", 30, 30);
        }
    }

    private void drawCar(Graphics2D g) {
        AffineTransform backup = g.getTransform();

        g.translate(getWidth() / 2, getHeight() - (CAR_LENGTH * PPM));
        g.setColor(Color.BLACK);
        g.fillRect(-(CAR_WIDTH * PPM / 2), 0, CAR_WIDTH * PPM, CAR_LENGTH * PPM);

        g.setTransform(backup);
    }

    private void drawTrackSensors(Graphics2D g) {
        AffineTransform backup = g.getTransform();

        g.translate(getWidth() / 2, getHeight() - (CAR_LENGTH * PPM));

        g.setColor(Color.RED);

        double[] sensors = new double[19];
        System.arraycopy(m.getTrackEdgeSensors(), 0, sensors, 0, 19);


        // coordinate system
        if (drawCoordinates) {
            g.setColor(Color.BLACK);
            // x-line
            g.drawLine(-(25 * PPM), 0, 25 * PPM, 0);
            g.drawString("X", (25 * PPM) - 5, 20);
            g.drawLine(-(20 * PPM), -5, -(20 * PPM), 5);
            g.drawString("-20m", -(20 * PPM) - 12, 20);
            g.drawLine(-(10 * PPM), -5, -(10 * PPM), 5);
            g.drawString("-10m", -(10 * PPM) - 12, 20);
            g.drawString("0m", -5, 20);
            g.drawLine((10 * PPM), -5, (10 * PPM), 5);
            g.drawString("10m", (10 * PPM) - 10, 20);
            g.drawLine((20 * PPM), -5, (20 * PPM), 5);
            g.drawString("20m", (20 * PPM) - 10, 20);
            // y line
            g.drawLine(0, PPM, 0, -220 * PPM);
            g.drawString("Y", 5, -220 * PPM);
            g.drawLine(-5, -50 * PPM, 5, -50 * PPM);
            g.drawString("50m", 20, -50 * PPM);
            g.drawLine(-5, -100 * PPM, 5, -100 * PPM);
            g.drawString("100m", 20, -100 * PPM);
            g.drawLine(-5, -150 * PPM, 5, -150 * PPM);
            g.drawString("150m", 20, -150 * PPM);
            g.drawLine(-5, -200 * PPM, 5, -200 * PPM);
            g.drawString("200m", 20, -200 * PPM);
        }

        int last_x = 0;
        int last_y = 0;

        int max_l = SensorData.maxTrackIndexLeft(m);
        int max_r = SensorData.maxTrackIndexRight(m);

        for (int i = 0; i < sensors.length; ++i) {
            double angle = Math.toRadians(angles[i]);
            int x_offset = (int) Math.round(Math.sin(angle) * sensors[i] * PPM);
            int y_offset = (int) Math.round(-Math.cos(angle) * sensors[i] * PPM);

            //g.setColor(Color.RED);
            //g.fillRect(x_offset - 1, y_offset - 1, 2, 2);
            if (drawSensors) {
                g.setColor(Color.BLUE);
                if (x_offset < 0) {
                    if (i % 2 == 0) {
                        g.drawString(Integer.toString(i), x_offset - 10, y_offset + (PPM));
                    } else {
                        g.drawString(Integer.toString(i), x_offset - 20, y_offset + (PPM));
                    }

                } else if (x_offset > 0) {
                    if (i % 2 == 0) {
                        g.drawString(Integer.toString(i), x_offset, y_offset);
                    } else {
                        g.drawString(Integer.toString(i), x_offset + 20, y_offset);
                    }

                } else {
                    g.drawString(Integer.toString(i), x_offset, y_offset - 20);
                }

                g.drawLine(0, 0, x_offset, y_offset);
            }

            if (i > 0 && drawOutline) {
                if (drawCapOff) {
                    if (i < max_l || i > max_r + 1) {
                        g.setColor(Color.GREEN);
                        if (i < max_l) {
                            int cor = 3;
                            if (i % 2 == 0) {
                                cor = -10;
                            }
                            g.drawString(String.valueOf(i - 1), last_x + ((x_offset - last_x) / 2) + cor, last_y + ((y_offset - last_y) / 2));

                        } else {
                            int z = 18 - i;
                            int cor = 3;
                            if (i % 2 == 0) {
                                cor = -10;
                            }
                            g.drawString(String.valueOf(z), last_x + ((x_offset - last_x) / 2) + cor, last_y + ((y_offset - last_y) / 2));
                        }

                    } else {
                        g.setColor(Color.RED);
                    }
                    g.drawLine(last_x, last_y, x_offset, y_offset);

                } else {
                    g.setColor(DARK_GREEN);
                    g.drawLine(last_x, last_y, x_offset, y_offset);
                }
            }


            last_x = x_offset;
            last_y = y_offset;
        }

        if (drawFocus) {
            g.setColor(PURPLE);
            for (int i = -2; i < 3; ++i) {
                double angle = Math.toRadians(focusAngle + i);
                int x_offset = (int) Math.round(Math.sin(angle) * focusSensors[i + 2] * PPM);
                int y_offset = (int) Math.round(-Math.cos(angle) * focusSensors[i + 2] * PPM);

                g.drawLine(0, 0, x_offset, y_offset);
            }
        }

        if (this.drawFiltered) {
            filter2(sensors);

            for (int i = 0; i < sensors.length; ++i) {
                double angle = Math.toRadians(angles[i]);
                int x_offset = (int) Math.round(Math.sin(angle) * sensors[i] * PPM);
                int y_offset = (int) Math.round(-Math.cos(angle) * sensors[i] * PPM);

                //g.setColor(Color.RED);
                //g.fillRect(x_offset - 1, y_offset - 1, 2, 2);

                g.setColor(Color.PINK);
                g.drawLine(0, 0, x_offset, y_offset);
            }
        }


        g.setTransform(backup);
    }

    private void filter1(double[] track) {

        // Find the indeces of the biggest sensor value
        int leftIndex = SensorData.maxTrackIndexLeft(m);
        int rightIndex = SensorData.maxTrackIndexRight(m);

        if (leftIndex > 2) {
            double[] x = new double[leftIndex];
            double[] y = new double[leftIndex];

            for (int i = 0; i < x.length; ++i) {
                x[i] = i * 1.0;
                y[i] = track[i];
            }

            Regression reg = new Regression(x, y);

            try {
                reg.polynomial(2);
                double[] values = reg.getYcalc();

                for (int i = 0; i < values.length; ++i) {
                    track[i] = values[i];
                }

            } catch (IllegalArgumentException e) {
                leftIndex = 0;
            }
        }

        if (rightIndex < track.length - 3) {
            double[] x = new double[track.length - (rightIndex + 1)];
            double[] y = new double[track.length - (rightIndex + 1)];

            for (int i = 0; i < x.length; ++i) {
                x[i] = i * 1.0;
                y[i] = track[track.length - (1 + i)];
            }

            Regression reg = new Regression(x, y);
            try {
                reg.polynomial(2);
                double[] values = reg.getYcalc();

                for (int i = 0; i < values.length; ++i) {
                    track[i] = values[i];
                }

            } catch (IllegalArgumentException e) {
                rightIndex = track.length;
            }
        }
    }

    private void filter2(double[] track) {
        // Calculate coordinates
        ModifiableSensorData data = new ModifiableSensorData();
        data.setTrackEdgeSensors(track);
        Point2D[] points = SensorData.calculateTrackPoints(data);

        // Find the indeces of the biggest sensor value
        int leftIndex = SensorData.maxTrackIndexLeft(data);
        int rightIndex = SensorData.maxTrackIndexRight(data);

        if (leftIndex > 2) {
            double[] x = new double[leftIndex];
            double[] y = new double[leftIndex];

            for (int i = 0; i < x.length; ++i) {
                x[i] = points[i].getY();
                y[i] = points[i].getX();
            }

            Regression reg = new Regression(x, y);

            try {
                reg.polynomial(2);
                double[] values = reg.getYcalc();

                for (int i = 0; i < values.length; ++i) {
                    points[i].setLocation(values[i], points[i].getY());
                    track[i] = Math.sqrt(values[i]*values[i]+points[i].getY()*points[i].getY());
                }

            } catch (IllegalArgumentException e) {
                leftIndex = 0;
            }
        }

        if (rightIndex < points.length - 3) {
            double[] x = new double[points.length - (rightIndex + 1)];
            double[] y = new double[points.length - (rightIndex + 1)];

            for (int i = 0; i < x.length; ++i) {
                x[i] = points[points.length - (1 + i)].getY();
                y[i] = points[points.length - (1 + i)].getX();
            }

            Regression reg = new Regression(x, y);
            try {
                reg.polynomial(2);
                double[] values = reg.getYcalc();

                for (int i = 0; i < values.length; ++i) {
                    points[points.length - (1 + i)].setLocation(values[i], points[points.length - (1 + i)].getY());

                    track[track.length - (1 + i)] = Math.sqrt(values[i]*values[i]+
                            points[points.length - (1 + i)].getY()*points[points.length - (1 + i)].getY());
                }

            } catch (IllegalArgumentException e) {
                rightIndex = points.length;
            }
        }
        
    }

    public void setDrawOutline(boolean b) {
        this.drawOutline = b;
        repaint();
    }

    public void setDrawCoordinates(boolean b) {
        this.drawCoordinates = b;
        repaint();
    }

    public void setDrawCapOff(boolean b) {
        this.drawCapOff = b;
        repaint();
    }

    public void setDrawSensors(boolean b) {
        this.drawSensors = b;
        repaint();
    }

    public void setDrawFocus(boolean b) {
        this.drawFocus = b;
        repaint();
    }

    public void setDrawFiltered(boolean b) {
        this.drawFiltered = b;
        repaint();
    }

    public void setSensorData(SensorData model, int focusAngle) {
        if (model.focusAvailable()) {
            this.focusAngle = focusAngle;
            System.arraycopy(model.getFocusSensors(), 0, this.focusSensors, 0, this.focusSensors.length);
        }
        this.m.setData(model);
        repaint();
    }
}
