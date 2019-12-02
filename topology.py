from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import CPULimitedHost
from mininet.link import TCLink
from mininet.util import dumpNodeConnections
from mininet.log import setLogLevel

class MyTopo( Topo ):
    "Simple topology example."

    def __init__( self ):
        "Create custom topo."

        # Initialize topology
        Topo.__init__( self )

        # Add hosts and switches
        
        Host1 = self.addHost( 'h1' )
        Host2 = self.addHost( 'h2' )
        Host3 = self.addHost( 'h3' )
        Host4 = self.addHost( 'h4' )
        Host5 = self.addHost( 'h5' )
        Switch = self.addSwitch( 's0' )

        # Add links
        linkopts = dict(bw=10, delay='5ms', loss=1, max_queue_size=1000, use_htb=True)
        self.addLink( Host1, Switch, **linkopts)
        self.addLink( Host2, Switch, **linkopts)
        self.addLink( Host3, Switch, **linkopts)
        self.addLink( Host4, Switch, **linkopts)
        self.addLink( Host5, Switch, **linkopts)

        


topos = { 'mytopo': ( lambda: MyTopo() ) }