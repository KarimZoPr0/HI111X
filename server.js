const net = require('net');
const clients = [];

const server = net.createServer(socket => {
    socket.setNoDelay(true);

    console.log('New client connected');
    clients.push(socket);

    socket.on('data', data => {
        for (let i = 0; i < clients.length; i++) {
            if (clients[i] !== socket && clients[i].writable) {
                clients[i].write(data);
            }
        }
    });

    socket.on('close', () => {
        const index = clients.indexOf(socket);
        if (index !== -1) {
            clients.splice(index, 1);
        }
        console.log('Client disconnected, remaining clients:', clients.length);
    });

    socket.on('error', err => {
        console.error('Socket error:', err.message);
        const index = clients.indexOf(socket);
        if (index !== -1) {
            clients.splice(index, 1);
        }
    });
});

server.listen(1234, '0.0.0.0', () => {
    console.log('TCP relay server running on port 1234');
});

server.on('error', err => {
    console.error('Server error:', err.message);
});