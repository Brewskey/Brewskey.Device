const fs = require('fs');
const path = require('path');

const startingDirectory = path.join(__dirname, 'Source');

const walkSync = function(dir, filelist) {
  const files = fs.readdirSync(dir);
  filelist = filelist || [];
  files.forEach(function(file) {
    const filePath = path.join(dir, file);
    if (fs.statSync(filePath).isDirectory()) {
      filelist = walkSync(filePath, filelist);
    } else {
      filelist.push(filePath.replace(__dirname + '\\', '').replace(/\\/g, '/'));
    }
  });
  return filelist;
};

walkSync(startingDirectory)
  .filter(file => file.endsWith('.h') || file.endsWith('.cpp'))
  .forEach(file => console.log(file));
