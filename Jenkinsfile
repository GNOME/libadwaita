pipeline {
    agent { docker 'godiug/librem5ci:latest' }
    stages {
        stage('Build') {
            steps {
                checkout scm
                sh 'rm -rf _build'
                sh 'meson . _build'
                sh 'ninja -C _build'
            }
        }
        stage('Test') {
            steps {
                sh 'xvfb-run ninja -C _build test'
            }
        }
    }
}
