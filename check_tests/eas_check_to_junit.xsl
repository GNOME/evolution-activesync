<?xml version="1.0" encoding="Windows-1252"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns:chk="http://check.sourceforge.net/ns">
    <xsl:output encoding="utf-8" indent="yes"/>

    <xsl:template match="chk:testsuites">
        <xsl:apply-templates select="chk:suite"/>
    </xsl:template>

    <xsl:template match="chk:suite">
        <testsuites>
        <xsl:for-each select="chk:test">
            <xsl:element name="testcase">
                <xsl:attribute name="classname">
                    <xsl:value-of select="chk:description"/>
                </xsl:attribute>
                <xsl:attribute name="name">
                    <xsl:value-of select="chk:id"/>
                </xsl:attribute>
            </xsl:element>
        </xsl:for-each>
        </testsuites>
    </xsl:template>


</xsl:stylesheet>
