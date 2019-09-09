const gulp = require('gulp');
const purify = require('gulp-purifycss');
const brotli = require('gulp-brotli');

gulp.task('purify-css', () => {
    return gulp.src('./dist/styles.css')
        .pipe(
            purify(
                ['./src/app/**/*.ts', './src/app/**/*.html'],
                {
                    info: true, // Outputs reduction information (like in the screenshot above)
                    minify: true, // Minifies the files after reduction
                    rejected: false, // Logs the CSS rules that were removed
                    whitelist: ['*transition*', '*dimmer*'] // Ignored css classes
                }
            ),
        )
        .pipe(gulp.dest('./dist/'));
});

gulp.task('compress', function() {
    return gulp.src(['./dist/*.js', './dist/*.css', './dist/*.html'])
        .pipe(brotli.compress({
            extension: 'br',
            skipLarger: true,
            mode: 1, // 0 = generic, 1 = text, 2 = font (WOFF2)
            quality: 10,
            lgwin: 12 // default
        }))
        .pipe(gulp.dest('./dist/spiffs'));
});

gulp.task('copy-icon', function() {
    return gulp.src('./dist/favicon.ico')
        .pipe(gulp.dest('./dist/spiffs'));
});
