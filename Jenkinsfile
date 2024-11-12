pipeline{

agent { dockerfile true }

stages{ 
    
    stage("build"){
        steps{ 
             echo 'building the application'
             sh ''' cd build
                  cmake .. 
                  make
            '''
            }
        }

     stage("test"){
        steps{ 
             echo 'testing the application'
             sh '''
                   cd build
                   cd test
                   ls
             '''
            }
        }

        stage("deploy"){
            steps{ 
             echo 'deploying the application'
             script{
              deploy();
             }
        } 
    }
}
}

def deploy(){

}