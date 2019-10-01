const gulp = require('gulp');
const purify = require('gulp-purifycss');
const gzip = require('gulp-gzip');

gulp.task('purify-css', () => {
    return gulp.src('./dist/styles.css')
        .pipe(
            purify(
                ['./dist/*.js', './dist/*.html'],
                {
                    info: true, // Outputs reduction information (like in the screenshot above)
                    minify: true, // Minifies the files after reduction
                    rejected: false, // Logs the CSS rules that were removed
                    whitelist: [] // Ignored css classes
                }
            ),
        )
        .pipe(gulp.dest('./dist'));
});

gulp.task('compress', function() {
    return gulp.src(['./dist/*.js', './dist/*.css'])
        .pipe(gzip())
        .pipe(gulp.dest('./dist/spiffs'));
});

gulp.task('copy-files', function() {
    return gulp.src(['./dist/index.html', './dist/favicon.ico'])
        .pipe(gulp.dest('./dist/spiffs'));
});
