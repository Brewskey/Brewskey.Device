const fs = require('fs');
const path = require('path');

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

walkSync(path.join(__dirname, 'Tests'))
  .filter(
    file =>
      file.endsWith('.h') || file.endsWith('.cpp') || file.endsWith('.hpp'),
  )
  .forEach(file => console.log(file));
walkSync(path.join(__dirname, 'Source'))
  .filter(
    file =>
      file.endsWith('.h') || file.endsWith('.cpp') || file.endsWith('.hpp'),
  )
  .forEach(file => console.log(file));
