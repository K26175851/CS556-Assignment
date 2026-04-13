import matplotlib.pyplot as plt
 
# -------------------------------

# GRAPH 1: TOTAL SERVICE TIME

# -------------------------------
 
methods = ['FIFO', 'VOQ Optimal', 'VOQ iSLIP']

service_time = [11, 7, 8]
 
plt.figure()

plt.bar(methods, service_time)
 
plt.xlabel('Scheduling Method')

plt.ylabel('Total Time Slots')

plt.title('Total Service Time Comparison')
 
plt.show()
 


# -------------------------------

# GRAPH 2: BACKLOG OVER TIME

# (Arrived - Served)

# -------------------------------
 
# Corrected backlog values
 
fifo_backlog = [4, 4, 4, 5, 6, 6, 5, 3, 2, 1, 0]

voq_opt_backlog = [2, 1, 0, 1, 1, 1, 0]

islip_backlog = [3, 3, 2, 3, 3, 4, 1, 0]
 
# Time axes

t_fifo = list(range(len(fifo_backlog)))

t_opt = list(range(len(voq_opt_backlog)))

t_islip = list(range(len(islip_backlog)))
 
plt.figure()
 
plt.plot(t_fifo, fifo_backlog, marker='o', label='FIFO')

plt.plot(t_opt, voq_opt_backlog, marker='o', label='VOQ Optimal')

plt.plot(t_islip, islip_backlog, marker='o', label='VOQ iSLIP')
 
plt.xlabel('Time Slots (t)')

plt.ylabel('Packets in Switch (Backlog)')

plt.title('Backlog Over Time (Arrived - Served)')
 
plt.legend()
 
plt.show()
 