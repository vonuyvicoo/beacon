import React, { useState } from "react";
import { Box, Button, Flex, Text, Link, useDisclosure } from "@chakra-ui/react";
import { FaGithub, FaMoon, FaSun } from "react-icons/fa";
import ProfileMenu from "./ProfileMenu";

const Navbar = (props) => {
  const { isOpen, onOpen, onClose } = useDisclosure();

  const colormode = localStorage.getItem("chakra-ui-color-mode");
  const [icon, seticon] = useState(
    colormode === "dark" ? <FaSun /> : <FaMoon />
  );

  const handleToggle = () => {
    if (colormode === "dark") {
      seticon(<FaMoon />);
      props.toggleColorMode();
    } else {
      seticon(<FaSun />);
      props.toggleColorMode();
    }
  };

  return (
    <>
      {/* Navbar for small screens */}
      {!window.location.pathname.includes("dashboard") && (
        <Box
          position={"absolute"}
          top={5}
          left={5}
          display={{ md: "none", base: "flex" }}
        >
          {/* Color mode toggle button */}
          <Button
            p={3}
            borderRadius={"full"}
            borderWidth={1}
            fontSize={"small"}
            backgroundColor={"transparent"}
            onClick={handleToggle}
            mx={1}
          >
            {icon}
          </Button>
        </Box>
      )}

      {/* Navbar for larger screens */}
      <Box
        p={3}
        w={{ base: "94vw", md: "99vw" }}
        m={2}
        borderRadius="10px"
        borderWidth="2px"
        display={{ base: "none", md: "block" }}
      >
        <Flex justify={"space-between"}>
          {/* Logo */}
          <Text fontSize="2xl">BEACON</Text>

          <Box
            display={{ base: "none", md: "block" }}
            justifyContent="space-between"
            alignItems="center"
          >
            {/* Color mode toggle button */}
            <Button
              onClick={handleToggle}
              mr={2}
              borderRadius={"full"}
              borderWidth={1}
              fontSize={"small"}
              backgroundColor={"transparent"}
              p={3}
            >
              {icon}
            </Button>
            {/* Profile menu */}
            {localStorage.getItem("token") && (
              <ProfileMenu
                context={props.context}
                isOpen={isOpen}
                onOpen={onOpen}
                onClose={onClose}
              />
            )}
          </Box>
        </Flex>
      </Box>
    </>
  );
};

export default Navbar;
