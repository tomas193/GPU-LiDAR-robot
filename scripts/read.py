#!/usr/bin/env python

import rospy
from sensor_msgs.msg import LaserScan

test=[]
def callback(data):
  test.append(str(data.ranges))
  rospy.signal_shutdown("mensaje recibido")

def inicio(test):
  rospy.init_node('lidar_listener',anonymous=True)
  rospy.Subscriber('/scan',LaserScan,callback)
  rospy.spin()
 
  datos=test[0]
  del test 
  datos=datos.replace('(','')
  datos=datos.replace(')','')
  datos=datos.split(',')
  return datos

datos=inicio(test)

cadena=''

for i in datos: 
  if float(i)==float('inf'):cadena+=str(0)+','
  else: cadena+=i.lstrip()+','

cadena=cadena[:-1]

with open('lectures.txt','w') as archivo:
  archivo.write(cadena)
