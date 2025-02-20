#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

# Test `ls` command runs without errors
@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

# Test executing an external command with arguments
@test "Running ls with -l flag" {
    run ./dsh <<EOF
ls -l
EOF

    [ "$status" -eq 0 ]
}

# Test handling `exit` command
@test "Exit command test" {
    run ./dsh <<EOF                
exit
EOF

    [ "$status" -eq 0 ]
}

