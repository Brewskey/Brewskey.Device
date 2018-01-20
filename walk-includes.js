const fs = require('fs');
const path = require('path');

const startingDirectory = path.join(__dirname, 'Source');

const walkSync = function(dir, dirs) {
  const files = fs.readdirSync(dir);
  dirs = dirs || [];
  dirs.push(dir.replace(__dirname + '\\', '').replace(/\\/g, '/'));

  files.forEach(function(file) {
    const filePath = path.join(dir, file);
    if (fs.statSync(filePath).isDirectory()) {
      dirs = walkSync(filePath, dirs);
    }
  });
  return dirs;
};

walkSync(startingDirectory).forEach(file => console.log(file));
