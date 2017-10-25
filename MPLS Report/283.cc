/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Andrey Churin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Andrey Churin <aachurin@gmail.com>
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/mpls-module.h"
#include "ns3/ipv4-global-routing-helper.h"

#include <iostream>

using namespace ns3;
using namespace mpls;

int
main (int argc, char *argv[])
{
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("mpls::MplsProtocol", LOG_LEVEL_DEBUG);
  LogComponentEnable ("mpls::Ipv4Routing", LOG_LEVEL_DEBUG);
  LogComponentEnable ("MplsNetworkDiscoverer", LOG_LEVEL_DEBUG);  

  NodeContainer hosts;
  NodeContainer routers;
  NodeContainer routers2;

  PointToPointHelper pointToPoint;
  Ipv4AddressHelper address;
  NetDeviceContainer devices;
  InternetStackHelper internet;
  MplsNetworkConfigurator network;
   routers = network.CreateAndInstall (8);
  hosts.Create (5);
  internet.Install (hosts);


  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("1ms"));
  
  // Hosts applications
  uint16_t port = 9;
  UdpEchoServerHelper server (port);
  ApplicationContainer apps = server.Install (hosts.Get(1));
  apps.Start (Seconds (0.0));
  apps.Stop (Seconds (10.0));

  UdpEchoClientHelper client (Ipv4Address("192.168.4.2"), port);
  client.SetAttribute ("MaxPackets", UintegerValue (1));
  client.SetAttribute ("Interval", TimeValue (MilliSeconds (0.00)));
  client.SetAttribute ("PacketSize", UintegerValue (1024));
  apps = client.Install (hosts.Get (0));
  apps = client.Install (hosts.Get (2));
  UdpEchoClientHelper client1 (Ipv4Address("192.168.5.2"), port);
  client1.SetAttribute ("MaxPackets", UintegerValue (1));
  client1.SetAttribute ("Interval", TimeValue (MilliSeconds (0.00)));
  client1.SetAttribute ("PacketSize", UintegerValue (1024));
  apps = client1.Install (hosts.Get (0));
  apps = client1.Install (hosts.Get (2));
  UdpEchoClientHelper client2 (Ipv4Address("192.168.6.2"), port);
  client2.SetAttribute ("MaxPackets", UintegerValue (1));
  client2.SetAttribute ("Interval", TimeValue (MilliSeconds (0.00)));
  client2.SetAttribute ("PacketSize", UintegerValue (1024));
  apps = client2.Install (hosts.Get (0));
  apps = client2.Install (hosts.Get (2));
  apps.Start (Seconds (0.0));
  apps.Stop (Seconds (2.0));

  // Hosts configuration
  devices = pointToPoint.Install (hosts.Get(0), routers.Get(0));
  address.SetBase ("192.168.1.0", "255.255.255.0");
  address.Assign (devices);

  devices = pointToPoint.Install (hosts.Get(2), routers.Get(1));
  address.SetBase ("192.168.2.0", "255.255.255.0");
  address.Assign (devices);


  // Routers configuration
  devices = pointToPoint.Install (routers.Get(0), routers.Get(2));
  address.SetBase ("10.1.1.0", "255.255.255.0");
  address.Assign (devices);

  devices = pointToPoint.Install (routers.Get(1), routers.Get(3));
  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (devices);

  devices = pointToPoint.Install (routers.Get(2), routers.Get(4));
  address.SetBase ("10.1.4.0", "255.255.255.0");
  address.Assign (devices);

  devices = pointToPoint.Install (routers.Get(3), routers.Get(4));
  address.SetBase ("10.1.5.0", "255.255.255.0");
  address.Assign (devices);

  devices = pointToPoint.Install (routers.Get(4), routers.Get(5));
  address.SetBase ("10.1.6.0", "255.255.255.0");
  address.Assign (devices);
  
  // These addresses need not to be specified in Label assignment
  devices = pointToPoint.Install (routers.Get(4), routers.Get(6));
  address.SetBase ("10.1.7.0", "255.255.255.0");
  address.Assign (devices);

  devices = pointToPoint.Install (routers.Get(4), routers.Get(7));
  address.SetBase ("10.1.8.0", "255.255.255.0");
  address.Assign (devices);
  
  // Address is not specified. Mpls interfaces will be disabled.
  devices = pointToPoint.Install (routers.Get(5), routers.Get(0));
  devices = pointToPoint.Install (routers.Get(6), routers.Get(0));
  devices = pointToPoint.Install (routers.Get(7), routers.Get(0));

  NhlfeSelectionPolicyHelper policy;
  policy.SetAttribute ("MaxPacketsInTxQueue", IntegerValue (0));
  
  MplsSwitch LSR1 (routers.Get (0));
  MplsSwitch LSR2 (routers.Get (1));
  MplsSwitch LSR3 (routers.Get (2));
  MplsSwitch LSR4 (routers.Get (3));
  MplsSwitch LSR5 (routers.Get (4));
  MplsSwitch LSR6 (routers.Get (5));
  MplsSwitch LSR7 (routers.Get (6));
  MplsSwitch LSR8 (routers.Get (7));
  LSR1.SetSelectionPolicy (policy);

  //Phase 2 needs to be done under this (Label Assignment)segment 
  LSR1.AddFtn (Ipv4Source ("192.168.1.1") && Ipv4Destination ("192.168.4.2"),
      Nhlfe (Swap (101), Ipv4Address ("10.1.1.2"))
  );
  LSR2.AddFtn (Ipv4Source ("192.168.2.1") && Ipv4Destination ("192.168.4.2"),
      Nhlfe (Swap (xxx), Ipv4Address ("xxx.xxx.xxx.xxx"))
  );
  LSR1.AddFtn (Ipv4Source ("192.168.1.1") && Ipv4Destination ("192.168.5.2"),
      Nhlfe (Swap (101), Ipv4Address ("10.1.1.2"))
  );
  LSR2.AddFtn (Ipv4Source ("192.168.2.1") && Ipv4Destination ("192.168.5.2"),
      Nhlfe (Swap (xxx), Ipv4Address ("xxx.xxx.xxx.xxx"))
  );
  LSR1.AddFtn (Ipv4Source ("192.168.1.1") && Ipv4Destination ("192.168.6.2"),
      Nhlfe (Swap (101), Ipv4Address ("10.1.1.2"))
  );
  LSR2.AddFtn (Ipv4Source ("192.168.2.1") && Ipv4Destination ("192.168.6.2"),
      Nhlfe (Swap (xxx), Ipv4Address ("xxx.xxx.xxx.xxx"))
  );
  LSR3.AddIlm (101, 
      Nhlfe(Swap (xxx), Ipv4Address ("xxx.xxx.xxx.xxx"))
  );
  LSR4.AddIlm (xxx, 
      Nhlfe(Swap (xxx), Ipv4Address ("xxx.xxx.xxx.xxx"))
  );
  LSR5.AddIlm (xxx, 
      Nhlfe(Swap (605), Ipv4Address ("xxx.xxx.xxx.xxx"))
  );
  LSR5.AddIlm (xxx, 
      Nhlfe(Swap (605), Ipv4Address ("xxx.xxx.xxx.xxx"))
  );

  LSR6.AddIlm (605, 
      Nhlfe(Pop ())
  );
  LSR7.AddIlm (605, 
      Nhlfe(Pop ())
  );
  LSR8.AddIlm (605, 
      Nhlfe(Pop ())
  );


  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  network.DiscoverNetwork ();
  network.ShowConfig ();
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}