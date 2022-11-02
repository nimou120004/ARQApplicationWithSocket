#include "netDevice-setup.h"
#include "ns3/wifi-module.h"

namespace ns3
{

NetDeviceSetup::NetDeviceSetup(){}
NetDeviceSetup::~NetDeviceSetup () {}

NetDeviceContainer NetDeviceSetup::ConfigureDevices (NodeContainer& nodes)
{

  int channelWidth = 80;
    std::string Vht = "7";
    /*
      Setting up net devices. With PHY & MAC using default settings.
    */
    WifiHelper wifiHelper;
    wifiHelper.SetStandard (WIFI_PHY_STANDARD_80211ax_5GHZ); // WIFI_PHY_STANDARD_80211g WIFI_STANDARD_80211n_2_4GHZ  WIFI_PHY_STANDARD_80211n_5GHZ  WIFI_PHY_STANDARD_80211ax_5GHZ or WIFI_PHY_STANDARD_80211ac
    // WIFI_PHY_STANDARD_80211ax_5GHZ, serverChannelWidth, 5, 55, 0, 60, 120 * channelRateFactor));
    //WIFI_PHY_STANDARD_80211ax_5GHZ, clientChannelWidth, 5, 45, 0, 50, 160 * channelRateFactor));

    YansWifiPhyHelper wifiPhyHelper =  YansWifiPhyHelper::Default ();
    wifiPhyHelper.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

    wifiPhyHelper.Set ("TxPowerStart", DoubleValue(13)); // 2,5 W or 33,98 dBm
    wifiPhyHelper.Set ("TxPowerEnd", DoubleValue(13)); // 2,5 W or 33,98 dBm
    //wifiPhyHelper.Set ("RxSensitivity", DoubleValue(-72));
    wifiPhyHelper.Set ("RxNoiseFigure", DoubleValue (7));
    wifiPhyHelper.Set ("ChannelWidth", UintegerValue (channelWidth));
    wifiPhyHelper.Set ("Frequency", UintegerValue(5180));
    wifiPhyHelper.Set ("Antennas", UintegerValue(1));
    wifiPhyHelper.Set ("ChannelNumber", UintegerValue(42));
    //wifiPhyHelper.Set ("TxGain", DoubleValue(0));
    //wifiPhyHelper.Set ("RxGain", DoubleValue(0));


    YansWifiChannelHelper wifiChannelHelper;
    wifiChannelHelper.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannelHelper.AddPropagationLoss ("ns3::FriisPropagationLossModel",
                                          "SystemLoss", DoubleValue(1),
                                          "Frequency", DoubleValue(5.18e9));
    wifiPhyHelper.SetChannel (wifiChannelHelper.Create ());

    /* 802.11ax
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue (oss.str ()),
                                "ControlMode", StringValue (oss.str ()));
  */

    ///*1 version 802.11 n
    wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                        "DataMode", StringValue ( "VhtMcs" + Vht),
                                        "ControlMode", StringValue ("VhtMcs0"));

    HtWifiMacHelper macHelper = HtWifiMacHelper::Default ();
    macHelper.SetType ("ns3::AdhocWifiMac");
    NetDeviceContainer devices = wifiHelper.Install (wifiPhyHelper, macHelper, nodes);
    return devices;
}

}
