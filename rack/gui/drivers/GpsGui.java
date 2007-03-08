/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
package rack.gui.drivers;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

import rack.gui.main.MapViewDrawContext;
import rack.gui.main.RackModuleGui;
import rack.main.*;
import rack.drivers.GpsDataMsg;
import rack.drivers.GpsProxy;

public class GpsGui extends RackModuleGui
{
    protected boolean       mapViewIsShowing=false;
    protected GpsDataMsg    gpsData;

    protected JButton onButton;
    protected JButton offButton;

    protected JPanel panel;
    protected JPanel buttonPanel;
    protected JPanel labelPanel;
    protected JPanel northPanel;

    protected JLabel modeLabel          = new JLabel();
    protected JLabel modeNameLabel      = new JLabel("Mode",
                                                     SwingConstants.RIGHT);
    protected JLabel latLabel           = new JLabel();
    protected JLabel latNameLabel       = new JLabel("Latitude",
                                                     SwingConstants.RIGHT);
    protected JLabel longLabel          = new JLabel();
    protected JLabel longNameLabel      = new JLabel("Longitude",
                                                     SwingConstants.RIGHT);
    protected JLabel altLabel           = new JLabel();
    protected JLabel altNameLabel       = new JLabel("Altitude",
                                                     SwingConstants.RIGHT);
    protected JLabel headLabel          = new JLabel();
    protected JLabel headNameLabel      = new JLabel("Heading",
                                                     SwingConstants.RIGHT);
    protected JLabel speedLabel         = new JLabel();
    protected JLabel speedNameLabel     = new JLabel("Speed",
                                                     SwingConstants.RIGHT);
    protected JLabel satNumLabel        = new JLabel();
    protected JLabel satNumNameLabel    = new JLabel("Satellites",
                                                     SwingConstants.RIGHT);
    protected JLabel pdopLabel          = new JLabel();
    protected JLabel pdopNameLabel      = new JLabel("PDOP",
                                                     SwingConstants.RIGHT);
    protected JLabel utcTimeLabel       = new JLabel();
    protected JLabel utcTimeNameLabel   = new JLabel("UTC Time",
                                                     SwingConstants.RIGHT);
    protected JLabel xGKLabel           = new JLabel();
    protected JLabel xGKNameLabel       = new JLabel("X",
                                                     SwingConstants.RIGHT);
    protected JLabel yGKLabel           = new JLabel();
    protected JLabel yGKNameLabel       = new JLabel("Y",
                                                     SwingConstants.RIGHT);
    protected JLabel zGKLabel           = new JLabel();
    protected JLabel zGKNameLabel       = new JLabel("Z",
                                                     SwingConstants.RIGHT);
    protected JLabel rhoGKLabel         = new JLabel();
    protected JLabel rhoGKNameLabel     = new JLabel("Rho",
                                                     SwingConstants.RIGHT);    
    protected JLabel varXYLabel         = new JLabel();
    protected JLabel varXYNameLabel     = new JLabel("Variance XY",
                                                     SwingConstants.RIGHT); 
    protected JLabel varZLabel          = new JLabel();
    protected JLabel varZNameLabel      = new JLabel("Variance Z",
                                                     SwingConstants.RIGHT);    
    protected JLabel varRhoLabel        = new JLabel();
    protected JLabel varRhoNameLabel    = new JLabel("Variance Rho",
                                                     SwingConstants.RIGHT);    
    public GpsProxy gps;

    public GpsGui(GpsProxy proxy)
    {
        gps = proxy;

        panel = new JPanel(new BorderLayout(2,2));
        panel.setBorder(BorderFactory.createEmptyBorder(4,4,4,4));

        buttonPanel = new JPanel(new GridLayout(0,2,4,2));
        labelPanel  = new JPanel(new GridLayout(0,2,8,0));
        northPanel  = new JPanel(new BorderLayout(2, 2));

        onButton = new JButton("On");
        offButton = new JButton("Off");

        onButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                gps.on();
            }
        });

        offButton.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                gps.off();
            }
        });

        buttonPanel.add(onButton);
        buttonPanel.add(offButton);
        northPanel.add(new JLabel("gps"),BorderLayout.NORTH);
        northPanel.add(buttonPanel,BorderLayout.CENTER);

        labelPanel.add(modeNameLabel);
        labelPanel.add(modeLabel);
        labelPanel.add(latNameLabel);
        labelPanel.add(latLabel);
        labelPanel.add(longNameLabel);
        labelPanel.add(longLabel);
        labelPanel.add(altNameLabel);
        labelPanel.add(altLabel);
        labelPanel.add(headNameLabel);
        labelPanel.add(headLabel);
        labelPanel.add(speedNameLabel);
        labelPanel.add(speedLabel);
        labelPanel.add(satNumNameLabel);
        labelPanel.add(satNumLabel);
        labelPanel.add(pdopNameLabel);
        labelPanel.add(pdopLabel);
        labelPanel.add(utcTimeNameLabel);
        labelPanel.add(utcTimeLabel);
        labelPanel.add(xGKNameLabel);
        labelPanel.add(xGKLabel);
        labelPanel.add(yGKNameLabel);
        labelPanel.add(yGKLabel);
        labelPanel.add(zGKNameLabel);
        labelPanel.add(zGKLabel);
        labelPanel.add(rhoGKNameLabel);
        labelPanel.add(rhoGKLabel);        
        labelPanel.add(varXYNameLabel);
        labelPanel.add(varXYLabel);      
        labelPanel.add(varZNameLabel);
        labelPanel.add(varZLabel);        
        labelPanel.add(varRhoNameLabel);
        labelPanel.add(varRhoLabel);        
        panel.add(northPanel,BorderLayout.NORTH);
        panel.add(labelPanel,BorderLayout.CENTER);
    }

    public JComponent getComponent()
    {
        return(panel);
    }

    public String getModuleName()
    {
        return("Gps");
    }
    public RackProxy getProxy()
    {
        return (gps);
    }

    public void run()
    {
        GpsDataMsg data;

        float latitude;
        float longitude;
        float heading;
        float altitude;
        float speed;


        while (terminate == false)
        {
            if(panel.isShowing())
            {
                data = gps.getData();

                if(data != null)
                {
                    gpsData    = data;

                    latitude   = (float)Math.toDegrees(data.latitude);
                    longitude  = (float)Math.toDegrees(data.longitude);
                    heading    = (float)Math.toDegrees(data.heading);
                    altitude   = (float)data.altitude / 1000;
                    speed      = (float)data.speed / 1000;

                    if (data.mode == GpsDataMsg.MODE_2D)
                    {
                        modeLabel.setText("2D");
                    }
                    if (data.mode == GpsDataMsg.MODE_3D)
                    {
                        modeLabel.setText("3D");
                    }
                    else
                    {
                        modeLabel.setText("invalid");
                    }

                    if (latitude >= 0.0)
                    {
                        latLabel.setText(latitude+" deg N");
                    }
                    else
                    {
                        latLabel.setText(latitude+" deg S");
                    }

                    if (longitude >= 0.0)
                    {
                        longLabel.setText(longitude+" deg E");
                    }
                    else
                    {
                        longLabel.setText(longitude+" deg W");
                    }

                    altLabel.setText(altitude+" m");
                    headLabel.setText(heading+" deg");
                    speedLabel.setText(speed+" m/s");

                    satNumLabel.setText(data.satelliteNum+"");
                    pdopLabel.setText(data.pdop+"");
                    utcTimeLabel.setText(data.utcTime+" s");
                    xGKLabel.setText(data.posGK.x+" mm");
                    yGKLabel.setText(data.posGK.y+" mm");
                    zGKLabel.setText(data.posGK.z+" mm");
                    rhoGKLabel.setText((float)Math.toDegrees(data.posGK.rho)+" deg");
                    varXYLabel.setText(data.varXY+" mm");
                    varZLabel.setText(data.varZ+" mm");
                    varRhoLabel.setText((float)Math.toDegrees(data.varRho)+" deg");

                    modeNameLabel.setEnabled(true);
                    modeLabel.setEnabled(true);
                    latNameLabel.setEnabled(true);
                    latLabel.setEnabled(true);
                    longNameLabel.setEnabled(true);
                    longLabel.setEnabled(true);
                    altNameLabel.setEnabled(true);
                    altLabel.setEnabled(true);
                    headNameLabel.setEnabled(true);
                    headLabel.setEnabled(true);
                    speedNameLabel.setEnabled(true);
                    speedLabel.setEnabled(true);
                    satNumNameLabel.setEnabled(true);
                    satNumLabel.setEnabled(true);
                    pdopNameLabel.setEnabled(true);
                    pdopLabel.setEnabled(true);
                    utcTimeNameLabel.setEnabled(true);
                    utcTimeLabel.setEnabled(true);
                    xGKNameLabel.setEnabled(true);
                    xGKLabel.setEnabled(true);
                    yGKNameLabel.setEnabled(true);
                    yGKLabel.setEnabled(true);
                    zGKNameLabel.setEnabled(true);
                    zGKLabel.setEnabled(true);
                    rhoGKNameLabel.setEnabled(true);
                    rhoGKLabel.setEnabled(true);
                    varXYNameLabel.setEnabled(true);
                    varXYLabel.setEnabled(true);
                    varZNameLabel.setEnabled(true);
                    varZLabel.setEnabled(true);
                    varRhoNameLabel.setEnabled(true);
                    varRhoLabel.setEnabled(true);
                }
                else
                {
                    modeNameLabel.setEnabled(false);
                    modeLabel.setEnabled(false);
                    latNameLabel.setEnabled(false);
                    latLabel.setEnabled(false);
                    longNameLabel.setEnabled(false);
                    longLabel.setEnabled(false);
                    altNameLabel.setEnabled(false);
                    altLabel.setEnabled(false);
                    headNameLabel.setEnabled(false);
                    headLabel.setEnabled(false);
                    speedNameLabel.setEnabled(false);
                    speedLabel.setEnabled(false);
                    satNumNameLabel.setEnabled(false);
                    satNumLabel.setEnabled(false);
                    pdopNameLabel.setEnabled(false);
                    pdopLabel.setEnabled(false);
                    utcTimeNameLabel.setEnabled(false);
                    utcTimeLabel.setEnabled(false);
                    xGKNameLabel.setEnabled(false);
                    xGKLabel.setEnabled(false);
                    yGKNameLabel.setEnabled(false);
                    yGKLabel.setEnabled(false);
                    zGKNameLabel.setEnabled(false);
                    zGKLabel.setEnabled(false);
                    rhoGKNameLabel.setEnabled(false);
                    rhoGKLabel.setEnabled(false);
                    varXYNameLabel.setEnabled(false);
                    varXYLabel.setEnabled(false);
                    varZNameLabel.setEnabled(false);
                    varZLabel.setEnabled(false);
                    varRhoNameLabel.setEnabled(false);
                    varRhoLabel.setEnabled(false);                    
                }
            }
            try
            {
                Thread.sleep(1000);
            }
            catch(InterruptedException e)
            {
            }
        }
    }

    public boolean hasMapView() 
    {
        return true;
    }
    

    public void paintMapView(MapViewDrawContext drawContext) 
    {
        mapViewIsShowing = true;
        
        if (gpsData == null) return;
        
        Graphics2D worldGraphics = drawContext.getWorldGraphics();

        worldGraphics.setColor(Color.ORANGE);
        worldGraphics.drawArc(gpsData.posGK.x - 10000, gpsData.posGK.y - 10000,
                              20000, 20000, 0, 360);
        worldGraphics.drawLine(gpsData.posGK.x, gpsData.posGK.y,
                gpsData.posGK.x + (int)(10000 * Math.cos(gpsData.posGK.rho)),
                gpsData.posGK.y + (int)(10000 * Math.sin(gpsData.posGK.rho)));
    }
}
