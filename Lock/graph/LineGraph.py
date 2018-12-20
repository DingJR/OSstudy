# encoding=utf-8
import matplotlib.pyplot as plt
from pylab import *                                 #支持中文
mpl.rcParams['font.sans-serif'] = ['SimHei']

names = ['1', '4', '8', '12','16','20']
x = range(len(names))
filename = ["spinlock_output","mutex_output","mymutex_output","twophase_output"]
fele = []
for i in filename:
    ff = open(i,"r")
    temp = ff.readlines()
    fele.append(temp)
for i in range(3):
    y = []
    for j in range(4):
        temp = []
        for ele in range(6):
            temp.append(float(fele[j][ele+i*6]))
        y.append(temp)

    #plt.plot(x, y, 'ro-')
    #plt.plot(x, y1, 'bo-')
    #pl.xlim(-1, 11)  # 限定横轴的范围
    plt.ylim(0, 25)  # 限定纵轴的范围
    plt.plot(x, y[0],  marker='o', ms=10,mec='r',mfc='w',label="Spinlock")
    plt.plot(x, y[1], marker='*', ms=10,mec='b',mfc='w',label="PthreadMutex")
    plt.plot(x, y[2], marker='v', ms=10,mec='g',mfc='w',label="Mymutex")
    plt.plot(x, y[3], marker='v', ms=10,mec='g',mfc='w',label="Twophaselock")
    plt.legend()  # 让图例生效
    plt.xticks(x, names, rotation=45)
    plt.margins(0)
    plt.subplots_adjust(bottom=0.15)
    plt.ylabel("Time(s)") #Y轴标签
    plt.xlabel("Thread Number") #Y轴标签
    plt.title("Performance") #标题
    savefig(str(i)+".png")
    plt.clf()
