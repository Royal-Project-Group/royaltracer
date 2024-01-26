const path = require('path');

module.exports = {
    mode: 'development', // or 'production' for production builds
    entry: './main.js', 
    output: {
        filename: 'bundle.js', 
        path: path.resolve(__dirname, './dist'), 
    },
    module: {
        rules: [
            {
                test: /\.js$/, 
                exclude: /node_modules/,
                use: {
                    loader: 'babel-loader',
                },
            },
        ],
    },
    devServer: {
        static: {
            directory: path.join(__dirname, './'),
            watch: true
        }
    }
};
